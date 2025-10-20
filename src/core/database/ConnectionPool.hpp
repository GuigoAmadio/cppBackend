#pragma once

#include "Connection.hpp"
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

namespace Core::Database {

/**
 * @brief RAII wrapper para conexões do pool.
 * 
 * Quando o objeto sai de escopo, a conexão volta automaticamente para o pool.
 * Isso garante que conexões nunca vazem mesmo com exceções.
 * 
 * @example
 *   {
 *       auto conn = pool.acquire();
 *       conn->execute("SELECT * FROM users");
 *   } // Conexão volta pro pool automaticamente aqui
 */
class PooledConnection {
public:
    PooledConnection(Connection* conn, class ConnectionPool* pool);
    ~PooledConnection();
    
    // Desabilitar cópia
    PooledConnection(const PooledConnection&) = delete;
    PooledConnection& operator=(const PooledConnection&) = delete;
    
    // Permitir movimentação
    PooledConnection(PooledConnection&& other) noexcept;
    PooledConnection& operator=(PooledConnection&& other) noexcept;
    
    // Acesso à conexão subjacente
    Connection* operator->() { return connection_; }
    Connection& operator*() { return *connection_; }
    
    bool isValid() const { return connection_ != nullptr; }

private:
    Connection* connection_;
    ConnectionPool* pool_;
    
    void release();
};

/**
 * @brief Pool de conexões PostgreSQL.
 * 
 * Mantém um pool de conexões reutilizáveis para evitar overhead
 * de criar/destruir conexões a cada request.
 * 
 * @features
 * - Thread-safe (múltiplas threads podem adquirir conexões)
 * - Lazy initialization (cria conexões sob demanda)
 * - Max connections (limita número total)
 * - Timeout (espera por conexão disponível)
 * - Health check (valida conexões antes de retornar)
 * 
 * @example
 *   ConnectionPool pool("host=localhost port=5433 dbname=test", 5, 20);
 *   
 *   auto conn = pool.acquire();
 *   auto result = conn->execute("SELECT * FROM users");
 */
class ConnectionPool {
public:
    /**
     * @brief Cria um pool de conexões.
     * 
     * @param connectionString String de conexão PostgreSQL
     * @param minConnections Número mínimo de conexões (criadas imediatamente)
     * @param maxConnections Número máximo de conexões
     * @param acquireTimeout Timeout em ms para adquirir conexão (0 = sem timeout)
     */
    ConnectionPool(
        const std::string& connectionString,
        size_t minConnections = 2,
        size_t maxConnections = 10,
        size_t acquireTimeout = 5000
    );
    
    ~ConnectionPool();
    
    // Desabilitar cópia
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    
    /**
     * @brief Adquire uma conexão do pool.
     * 
     * Se não houver conexões disponíveis:
     * 1. Tenta criar nova (se < maxConnections)
     * 2. Espera uma ficar disponível (até timeout)
     * 3. Lança exceção se timeout
     * 
     * @return RAII wrapper que devolve conexão automaticamente
     * @throws std::runtime_error se timeout ou erro na criação
     */
    PooledConnection acquire();
    
    /**
     * @brief Retorna uma conexão para o pool.
     * 
     * Chamado automaticamente pelo PooledConnection destructor.
     * Você normalmente não precisa chamar isso diretamente.
     */
    void release(Connection* conn);
    
    /**
     * @brief Estatísticas do pool.
     */
    size_t availableConnections() const;
    size_t totalConnections() const;
    size_t activeConnections() const;
    
    /**
     * @brief Fecha todas as conexões.
     */
    void shutdown();

private:
    std::string connectionString_;
    size_t minConnections_;
    size_t maxConnections_;
    size_t acquireTimeout_;
    
    std::queue<Connection*> available_;
    std::atomic<size_t> totalConnections_{0};
    std::atomic<bool> shutdown_{false};
    
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    
    // Cria nova conexão (já adquire lock antes de chamar)
    Connection* createConnection();
    
    // Valida se conexão ainda está viva
    bool validateConnection(Connection* conn);
};

} // namespace Core::Database

