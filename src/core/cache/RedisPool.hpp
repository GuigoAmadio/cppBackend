#pragma once

#include "RedisClient.hpp"
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace Core::Cache {

/**
 * @brief Pool de conexões Redis
 * 
 * Gerencia um pool de conexões Redis para uso eficiente e thread-safe
 * Similar ao ConnectionPool do PostgreSQL
 */
class RedisPool {
public:
    /**
     * @brief Construtor
     * @param host Host do Redis
     * @param port Porta do Redis
     * @param poolSize Número de conexões no pool
     * @param timeout Timeout para acquire
     */
    explicit RedisPool(
        const std::string& host = "127.0.0.1",
        int port = 6379,
        size_t poolSize = 10,
        int timeoutSeconds = 5
    );
    
    /**
     * @brief Destrutor - fecha todas as conexões
     */
    ~RedisPool();
    
    // Delete copy
    RedisPool(const RedisPool&) = delete;
    RedisPool& operator=(const RedisPool&) = delete;
    
    /**
     * @brief Adquire uma conexão do pool
     * @param timeoutMs Timeout em ms (0 = aguarda indefinidamente)
     * @return RedisClient ou nullptr se timeout
     */
    std::unique_ptr<RedisClient> acquire(int timeoutMs = 5000);
    
    /**
     * @brief Retorna uma conexão ao pool
     * @param client Conexão a retornar
     */
    void release(std::unique_ptr<RedisClient> client);
    
    /**
     * @brief Retorna estatísticas do pool
     */
    struct Stats {
        size_t total;       // Total de conexões
        size_t available;   // Conexões disponíveis
        size_t inUse;       // Conexões em uso
    };
    
    Stats getStats() const;
    
    /**
     * @brief Testa todas as conexões (PING)
     * @return número de conexões válidas
     */
    size_t healthCheck();

private:
    std::string host_;
    int port_;
    size_t poolSize_;
    int timeoutSeconds_;
    
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::unique_ptr<RedisClient>> availableConnections_;
    size_t totalConnections_;
    
    /**
     * @brief Cria uma nova conexão
     */
    std::unique_ptr<RedisClient> createConnection();
    
    /**
     * @brief Valida se conexão está ok
     */
    bool validateConnection(RedisClient* client);
};

/**
 * @brief RAII guard para auto-release de conexão
 */
class RedisConnectionGuard {
public:
    RedisConnectionGuard(RedisPool& pool, std::unique_ptr<RedisClient> client)
        : pool_(pool), client_(std::move(client)) {}
    
    ~RedisConnectionGuard() {
        if (client_) {
            pool_.release(std::move(client_));
        }
    }
    
    // Delete copy
    RedisConnectionGuard(const RedisConnectionGuard&) = delete;
    RedisConnectionGuard& operator=(const RedisConnectionGuard&) = delete;
    
    // Allow move
    RedisConnectionGuard(RedisConnectionGuard&& other) noexcept
        : pool_(other.pool_), client_(std::move(other.client_)) {}
    
    RedisClient* get() { return client_.get(); }
    RedisClient* operator->() { return client_.get(); }
    
private:
    RedisPool& pool_;
    std::unique_ptr<RedisClient> client_;
};

} // namespace Core::Cache

