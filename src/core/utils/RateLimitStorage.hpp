#pragma once

#include <string>
#include <map>
#include <chrono>
#include <mutex>

namespace Core::Utils {

/**
 * @brief Estrutura para armazenar informações de rate limit
 */
struct RateLimitEntry {
    int count;
    std::chrono::system_clock::time_point windowStart;
    std::chrono::system_clock::time_point lastRequest;
};

/**
 * @brief Storage thread-safe para rate limiting
 * 
 * Implementa sliding window counter para rate limiting.
 * Usa map em memória - para produção considerar Redis.
 */
class RateLimitStorage {
public:
    RateLimitStorage();
    
    /**
     * @brief Verifica se key excedeu o limite
     * @param key Identificador (IP ou user_id)
     * @param limit Número máximo de requests
     * @param windowSeconds Janela de tempo em segundos
     * @return true se pode prosseguir, false se excedeu limite
     */
    bool checkLimit(const std::string& key, int limit, int windowSeconds);
    
    /**
     * @brief Registra um request para a key
     * @param key Identificador
     */
    void recordRequest(const std::string& key);
    
    /**
     * @brief Obtém número de requests restantes
     * @param key Identificador
     * @param limit Limite máximo
     * @param windowSeconds Janela de tempo
     * @return Número de requests restantes
     */
    int getRemainingRequests(const std::string& key, int limit, int windowSeconds);
    
    /**
     * @brief Obtém tempo até reset (em segundos)
     * @param key Identificador
     * @param windowSeconds Janela de tempo
     * @return Segundos até reset, ou 0 se já resetou
     */
    int getResetTime(const std::string& key, int windowSeconds);
    
    /**
     * @brief Remove entradas expiradas (limpeza)
     * @param windowSeconds Janela de expiração
     */
    void cleanup(int windowSeconds);
    
    /**
     * @brief Limpa todos os dados (para testes)
     */
    void clear();

private:
    std::map<std::string, RateLimitEntry> storage_;
    std::mutex mutex_;
    
    bool isWindowExpired(const RateLimitEntry& entry, int windowSeconds);
};

} // namespace Core::Utils

