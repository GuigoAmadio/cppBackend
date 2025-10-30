#include "RedisPool.hpp"
#include "../utils/LoggerNew.hpp"

namespace Core::Cache {

RedisPool::RedisPool(const std::string& host, int port, size_t poolSize, int timeoutSeconds)
    : host_(host),
      port_(port),
      poolSize_(poolSize),
      timeoutSeconds_(timeoutSeconds),
      totalConnections_(0) {
    
    LOG_INFO("RedisPool: Initializing pool with " + std::to_string(poolSize) + " connections");
    
    // Criar conexões iniciais
    for (size_t i = 0; i < poolSize; ++i) {
        try {
            auto client = createConnection();
            if (client && client->isConnected()) {
                std::lock_guard<std::mutex> lock(mutex_);
                availableConnections_.push(std::move(client));
                totalConnections_++;
            } else {
                LOG_WARNING("RedisPool: Failed to create connection " + std::to_string(i + 1));
            }
        } catch (const std::exception& e) {
            LOG_ERROR("RedisPool: Exception creating connection: " + std::string(e.what()));
        }
    }
    
    LOG_INFO("RedisPool: Initialized with " + std::to_string(totalConnections_) + " connections");
}

RedisPool::~RedisPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Limpar todas as conexões disponíveis
    while (!availableConnections_.empty()) {
        availableConnections_.pop();
    }
    
    totalConnections_ = 0;
    LOG_INFO("RedisPool: Destroyed");
}

std::unique_ptr<RedisClient> RedisPool::acquire(int timeoutMs) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // Aguardar até ter conexão disponível ou timeout
    if (timeoutMs == 0) {
        // Aguardar indefinidamente
        cv_.wait(lock, [this] { return !availableConnections_.empty(); });
    } else {
        // Aguardar com timeout
        auto timeout = std::chrono::milliseconds(timeoutMs);
        if (!cv_.wait_for(lock, timeout, [this] { return !availableConnections_.empty(); })) {
            LOG_WARNING("RedisPool: Timeout acquiring connection");
            return nullptr;
        }
    }
    
    // Pegar conexão disponível
    if (availableConnections_.empty()) {
        return nullptr;
    }
    
    auto client = std::move(availableConnections_.front());
    availableConnections_.pop();
    
    // Validar conexão antes de retornar
    if (!validateConnection(client.get())) {
        LOG_WARNING("RedisPool: Connection invalid, creating new one");
        client = createConnection();
    }
    
    return client;
}

void RedisPool::release(std::unique_ptr<RedisClient> client) {
    if (!client) {
        return;
    }
    
    // Validar conexão antes de devolver ao pool
    if (!validateConnection(client.get())) {
        LOG_WARNING("RedisPool: Discarding invalid connection");
        try {
            // Tentar reconectar
            if (client->reconnect() && validateConnection(client.get())) {
                LOG_INFO("RedisPool: Reconnected successfully");
            } else {
                // Criar nova conexão
                client = createConnection();
            }
        } catch (...) {
            client = createConnection();
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        availableConnections_.push(std::move(client));
    }
    
    // Notificar threads aguardando
    cv_.notify_one();
}

RedisPool::Stats RedisPool::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Stats stats;
    stats.total = totalConnections_;
    stats.available = availableConnections_.size();
    stats.inUse = totalConnections_ - availableConnections_.size();
    
    return stats;
}

size_t RedisPool::healthCheck() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t validConnections = 0;
    std::queue<std::unique_ptr<RedisClient>> validQueue;
    
    // Testar todas as conexões
    while (!availableConnections_.empty()) {
        auto client = std::move(availableConnections_.front());
        availableConnections_.pop();
        
        if (validateConnection(client.get())) {
            validConnections++;
            validQueue.push(std::move(client));
        } else {
            LOG_WARNING("RedisPool: Found invalid connection during health check");
            // Tentar criar nova
            try {
                auto newClient = createConnection();
                if (newClient && newClient->isConnected()) {
                    validQueue.push(std::move(newClient));
                    validConnections++;
                }
            } catch (...) {
                LOG_ERROR("RedisPool: Failed to create replacement connection");
            }
        }
    }
    
    // Restaurar fila com conexões válidas
    availableConnections_ = std::move(validQueue);
    
    LOG_INFO("RedisPool: Health check - " + std::to_string(validConnections) + "/" + 
             std::to_string(totalConnections_) + " valid connections");
    
    return validConnections;
}

std::unique_ptr<RedisClient> RedisPool::createConnection() {
    try {
        auto client = std::make_unique<RedisClient>(host_, port_, timeoutSeconds_);
        
        if (!client->isConnected()) {
            LOG_ERROR("RedisPool: Failed to connect to Redis");
            return nullptr;
        }
        
        return client;
    } catch (const std::exception& e) {
        LOG_ERROR("RedisPool: Exception creating connection: " + std::string(e.what()));
        return nullptr;
    }
}

bool RedisPool::validateConnection(RedisClient* client) {
    if (!client) {
        return false;
    }
    
    if (!client->isConnected()) {
        return false;
    }
    
    // Testar PING
    try {
        return client->ping();
    } catch (...) {
        return false;
    }
}

} // namespace Core::Cache

