#include "RedisCacheService.hpp"
#include "../utils/LoggerNew.hpp"

namespace Core::Cache {

RedisCacheService::RedisCacheService(std::shared_ptr<RedisPool> pool)
    : pool_(pool) {
    if (!pool_) {
        throw std::invalid_argument("RedisPool cannot be null");
    }
    LOG_INFO("RedisCacheService: Initialized");
}

// ========================================
// STRING OPERATIONS
// ========================================

bool RedisCacheService::set(const std::string& key, const std::string& value, int ttl) {
    auto client = pool_->acquire();
    if (!client) {
        LOG_ERROR("RedisCacheService: Failed to acquire connection");
        return false;
    }
    
    bool success;
    if (ttl > 0) {
        success = client->setex(key, ttl, value);
    } else {
        success = client->set(key, value);
    }
    
    pool_->release(std::move(client));
    return success;
}

std::optional<std::string> RedisCacheService::get(const std::string& key) {
    auto client = pool_->acquire();
    if (!client) {
        LOG_ERROR("RedisCacheService: Failed to acquire connection");
        return std::nullopt;
    }
    
    auto value = client->get(key);
    pool_->release(std::move(client));
    return value;
}

bool RedisCacheService::del(const std::string& key) {
    auto client = pool_->acquire();
    if (!client) {
        LOG_ERROR("RedisCacheService: Failed to acquire connection");
        return false;
    }
    
    bool success = client->del(key);
    pool_->release(std::move(client));
    return success;
}

bool RedisCacheService::exists(const std::string& key) {
    auto client = pool_->acquire();
    if (!client) {
        LOG_ERROR("RedisCacheService: Failed to acquire connection");
        return false;
    }
    
    bool result = client->exists(key);
    pool_->release(std::move(client));
    return result;
}

void RedisCacheService::flush() {
    auto client = pool_->acquire();
    if (!client) {
        LOG_ERROR("RedisCacheService: Failed to acquire connection");
        return;
    }
    
    client->flushdb();
    pool_->release(std::move(client));
    LOG_INFO("RedisCacheService: Cache flushed");
}

// ========================================
// HASH OPERATIONS
// ========================================

bool RedisCacheService::hset(const std::string& key, const std::string& field, const std::string& value) {
    auto client = pool_->acquire();
    if (!client) {
        LOG_ERROR("RedisCacheService: Failed to acquire connection");
        return false;
    }
    
    bool success = client->hset(key, field, value);
    pool_->release(std::move(client));
    return success;
}

std::optional<std::string> RedisCacheService::hget(const std::string& key, const std::string& field) {
    auto client = pool_->acquire();
    if (!client) {
        LOG_ERROR("RedisCacheService: Failed to acquire connection");
        return std::nullopt;
    }
    
    auto value = client->hget(key, field);
    pool_->release(std::move(client));
    return value;
}

// ========================================
// TTL OPERATIONS
// ========================================

bool RedisCacheService::expire(const std::string& key, int seconds) {
    auto client = pool_->acquire();
    if (!client) {
        LOG_ERROR("RedisCacheService: Failed to acquire connection");
        return false;
    }
    
    bool success = client->expire(key, seconds);
    pool_->release(std::move(client));
    return success;
}

int RedisCacheService::ttl(const std::string& key) {
    auto client = pool_->acquire();
    if (!client) {
        LOG_ERROR("RedisCacheService: Failed to acquire connection");
        return -2;
    }
    
    int ttl = client->ttl(key);
    pool_->release(std::move(client));
    return ttl;
}

// ========================================
// REDIS-SPECIFIC OPERATIONS
// ========================================

RedisPool::Stats RedisCacheService::getPoolStats() const {
    return pool_->getStats();
}

size_t RedisCacheService::healthCheck() {
    return pool_->healthCheck();
}

} // namespace Core::Cache

