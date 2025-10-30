#pragma once

#include "CacheService.hpp"
#include "RedisPool.hpp"
#include <memory>

namespace Core::Cache {

/**
 * @brief Implementação de CacheService usando Redis
 * 
 * Usa RedisPool para gerenciar conexões de forma eficiente
 */
class RedisCacheService : public CacheService {
public:
    /**
     * @brief Construtor
     * @param pool Pool de conexões Redis compartilhado
     */
    explicit RedisCacheService(std::shared_ptr<RedisPool> pool);
    
    ~RedisCacheService() override = default;
    
    // STRING OPERATIONS
    bool set(const std::string& key, const std::string& value, int ttl = 0) override;
    std::optional<std::string> get(const std::string& key) override;
    bool del(const std::string& key) override;
    bool exists(const std::string& key) override;
    void flush() override;
    
    // HASH OPERATIONS
    bool hset(const std::string& key, const std::string& field, const std::string& value) override;
    std::optional<std::string> hget(const std::string& key, const std::string& field) override;
    
    // TTL OPERATIONS
    bool expire(const std::string& key, int seconds) override;
    int ttl(const std::string& key) override;
    
    // ========================================
    // REDIS-SPECIFIC OPERATIONS
    // ========================================
    
    /**
     * @brief Retorna estatísticas do pool
     */
    RedisPool::Stats getPoolStats() const;
    
    /**
     * @brief Executa health check no pool
     */
    size_t healthCheck();

private:
    std::shared_ptr<RedisPool> pool_;
};

} // namespace Core::Cache

