#include "ConnectionPool.hpp"
#include "../utils/Logger.hpp"
#include <stdexcept>

namespace Core::Database {

// ==================== PooledConnection ====================

PooledConnection::PooledConnection(Connection* conn, ConnectionPool* pool)
    : connection_(conn), pool_(pool) {}

PooledConnection::~PooledConnection() {
    release();
}

PooledConnection::PooledConnection(PooledConnection&& other) noexcept
    : connection_(other.connection_), pool_(other.pool_) {
    other.connection_ = nullptr;
    other.pool_ = nullptr;
}

PooledConnection& PooledConnection::operator=(PooledConnection&& other) noexcept {
    if (this != &other) {
        release();
        connection_ = other.connection_;
        pool_ = other.pool_;
        other.connection_ = nullptr;
        other.pool_ = nullptr;
    }
    return *this;
}

void PooledConnection::release() {
    if (connection_ && pool_) {
        pool_->release(connection_);
        connection_ = nullptr;
        pool_ = nullptr;
    }
}

// ==================== ConnectionPool ====================

ConnectionPool::ConnectionPool(
    const std::string& connectionString,
    size_t minConnections,
    size_t maxConnections,
    size_t acquireTimeout
) : connectionString_(connectionString),
    minConnections_(minConnections),
    maxConnections_(maxConnections),
    acquireTimeout_(acquireTimeout) {
    
    if (minConnections > maxConnections) {
        throw std::invalid_argument("minConnections cannot be greater than maxConnections");
    }
    
    Utils::Logger::info("🏊 Criando connection pool (min=" + std::to_string(minConnections) + 
                       ", max=" + std::to_string(maxConnections) + ")");
    
    // Criar conexões mínimas
    std::lock_guard<std::mutex> lock(mutex_);
    for (size_t i = 0; i < minConnections; ++i) {
        try {
            Connection* conn = createConnection();
            available_.push(conn);
        } catch (const std::exception& e) {
            Utils::Logger::error("Falha ao criar conexão inicial: " + std::string(e.what()));
            // Continua tentando criar as outras
        }
    }
    
    Utils::Logger::info("✅ Connection pool criado com " + std::to_string(available_.size()) + " conexões");
}

ConnectionPool::~ConnectionPool() {
    shutdown();
}

PooledConnection ConnectionPool::acquire() {
    if (shutdown_) {
        throw std::runtime_error("Connection pool is shutdown");
    }
    
    std::unique_lock<std::mutex> lock(mutex_);
    
    auto startTime = std::chrono::steady_clock::now();
    
    while (true) {
        // 1. Se tem conexão disponível, usar
        if (!available_.empty()) {
            Connection* conn = available_.front();
            available_.pop();
            
            // Validar conexão antes de retornar
            if (validateConnection(conn)) {
                Utils::Logger::debug("📥 Conexão adquirida do pool (disponíveis: " + 
                                   std::to_string(available_.size()) + ")");
                return PooledConnection(conn, this);
            } else {
                // Conexão morreu, descartar
                Utils::Logger::warning("⚠️ Conexão inválida descartada");
                delete conn;
                totalConnections_--;
                continue;
            }
        }
        
        // 2. Se pode criar mais conexões, criar
        if (totalConnections_ < maxConnections_) {
            try {
                Connection* conn = createConnection();
                Utils::Logger::debug("🆕 Nova conexão criada (total: " + 
                                   std::to_string(totalConnections_) + ")");
                return PooledConnection(conn, this);
            } catch (const std::exception& e) {
                Utils::Logger::error("Falha ao criar conexão: " + std::string(e.what()));
                throw;
            }
        }
        
        // 3. Esperar por conexão disponível (com timeout)
        if (acquireTimeout_ == 0) {
            // Sem timeout, esperar indefinidamente
            Utils::Logger::debug("⏳ Aguardando conexão disponível...");
            cv_.wait(lock, [this] { return !available_.empty() || shutdown_; });
        } else {
            // Com timeout
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime
            ).count();
            
            if (elapsed >= acquireTimeout_) {
                throw std::runtime_error(
                    "Timeout ao adquirir conexão do pool (timeout=" + 
                    std::to_string(acquireTimeout_) + "ms)"
                );
            }
            
            auto remainingTime = std::chrono::milliseconds(acquireTimeout_ - elapsed);
            Utils::Logger::debug("⏳ Aguardando conexão (timeout: " + 
                               std::to_string(remainingTime.count()) + "ms)");
            
            cv_.wait_for(lock, remainingTime, [this] { 
                return !available_.empty() || shutdown_; 
            });
        }
        
        if (shutdown_) {
            throw std::runtime_error("Connection pool is shutdown");
        }
    }
}

void ConnectionPool::release(Connection* conn) {
    if (!conn) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (shutdown_) {
        // Pool em shutdown, descartar conexão
        delete conn;
        totalConnections_--;
        return;
    }
    
    // Validar conexão antes de devolver ao pool
    if (validateConnection(conn)) {
        available_.push(conn);
        Utils::Logger::debug("📤 Conexão devolvida ao pool (disponíveis: " + 
                           std::to_string(available_.size()) + ")");
        cv_.notify_one();
    } else {
        // Conexão morreu, descartar
        Utils::Logger::warning("⚠️ Conexão inválida descartada ao devolver");
        delete conn;
        totalConnections_--;
    }
}

size_t ConnectionPool::availableConnections() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return available_.size();
}

size_t ConnectionPool::totalConnections() const {
    return totalConnections_;
}

size_t ConnectionPool::activeConnections() const {
    return totalConnections_ - availableConnections();
}

void ConnectionPool::shutdown() {
    Utils::Logger::info("🛑 Shutting down connection pool...");
    
    std::lock_guard<std::mutex> lock(mutex_);
    shutdown_ = true;
    
    // Fechar todas as conexões disponíveis
    while (!available_.empty()) {
        Connection* conn = available_.front();
        available_.pop();
        delete conn;
        totalConnections_--;
    }
    
    cv_.notify_all();
    
    Utils::Logger::info("✅ Connection pool shutdown complete");
}

// ==================== PRIVATE ====================

Connection* ConnectionPool::createConnection() {
    // NOTA: Deve ser chamado com lock já adquirido
    try {
        Connection* conn = new Connection(connectionString_);
        totalConnections_++;
        return conn;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create connection: " + std::string(e.what()));
    }
}

bool ConnectionPool::validateConnection(Connection* conn) {
    if (!conn) return false;
    
    try {
        // Tentar executar query simples para validar
        if (!conn->isConnected()) {
            return false;
        }
        
        // Query rápida de validação
        auto result = conn->execute("SELECT 1");
        return result.isSuccess();
        
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace Core::Database

