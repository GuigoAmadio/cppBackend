#include "RateLimitStorage.hpp"
#include "LoggerNew.hpp"

namespace Core::Utils {

RateLimitStorage::RateLimitStorage() {
    LOG_DEBUG("RateLimitStorage initialized");
}

bool RateLimitStorage::isWindowExpired(const RateLimitEntry& entry, int windowSeconds) {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - entry.windowStart).count();
    return elapsed >= windowSeconds;
}

bool RateLimitStorage::checkLimit(const std::string& key, int limit, int windowSeconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::system_clock::now();
    
    // Se não existe entrada, criar nova
    if (storage_.find(key) == storage_.end()) {
        storage_[key] = RateLimitEntry{0, now, now};
        return true;
    }
    
    auto& entry = storage_[key];
    
    // Se janela expirou, resetar contador
    if (isWindowExpired(entry, windowSeconds)) {
        entry.count = 0;
        entry.windowStart = now;
        entry.lastRequest = now;
        return true;
    }
    
    // Verificar se ainda não excedeu limite
    if (entry.count >= limit) {
        LOG_DEBUG("Rate limit exceeded for key: " + key + " (" + std::to_string(entry.count) + "/" + std::to_string(limit) + ")");
        return false;
    }
    
    return true;
}

void RateLimitStorage::recordRequest(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::system_clock::now();
    
    if (storage_.find(key) == storage_.end()) {
        storage_[key] = RateLimitEntry{1, now, now};
    } else {
        storage_[key].count++;
        storage_[key].lastRequest = now;
    }
}

int RateLimitStorage::getRemainingRequests(const std::string& key, int limit, int windowSeconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (storage_.find(key) == storage_.end()) {
        return limit;
    }
    
    auto& entry = storage_[key];
    
    // Se janela expirou, limite completo disponível
    if (isWindowExpired(entry, windowSeconds)) {
        return limit;
    }
    
    int remaining = limit - entry.count;
    return remaining > 0 ? remaining : 0;
}

int RateLimitStorage::getResetTime(const std::string& key, int windowSeconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (storage_.find(key) == storage_.end()) {
        return 0;
    }
    
    auto& entry = storage_[key];
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - entry.windowStart).count();
    
    int resetTime = windowSeconds - elapsed;
    return resetTime > 0 ? resetTime : 0;
}

void RateLimitStorage::cleanup(int windowSeconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::system_clock::now();
    int removed = 0;
    
    for (auto it = storage_.begin(); it != storage_.end(); ) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.lastRequest).count();
        
        // Remove entradas que não tiveram atividade há mais de 2x o window
        if (elapsed >= windowSeconds * 2) {
            it = storage_.erase(it);
            removed++;
        } else {
            ++it;
        }
    }
    
    if (removed > 0) {
        LOG_DEBUG("Rate limit cleanup: removed " + std::to_string(removed) + " expired entries");
    }
}

void RateLimitStorage::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    storage_.clear();
    LOG_DEBUG("Rate limit storage cleared");
}

} // namespace Core::Utils

