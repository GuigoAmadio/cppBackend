#pragma once

#include "Middleware.hpp"
#include "../utils/RateLimitStorage.hpp"
#include <memory>
#include <string>

namespace Core::Http {

/**
 * @brief Configuração para rate limiting
 */
struct RateLimitConfig {
    int maxRequests;       // Número máximo de requests
    int windowSeconds;     // Janela de tempo em segundos
    bool byIp;            // Limitar por IP
    bool byUser;          // Limitar por usuário autenticado
    std::string message;  // Mensagem customizada
    
    RateLimitConfig(int max, int window, bool ip = true, bool user = false, const std::string& msg = "")
        : maxRequests(max), windowSeconds(window), byIp(ip), byUser(user), message(msg) {}
};

/**
 * @brief Factory para criar rate limit middleware
 * 
 * @example Básico (por IP):
 *   auto rateLimitMw = createRateLimitMiddleware(10, 60); // 10 req/min
 * 
 * @example Por usuário autenticado:
 *   RateLimitConfig config(100, 3600, false, true); // 100 req/hora por user
 *   auto rateLimitMw = createRateLimitMiddleware(config);
 * 
 * @example Combinado (IP + usuário):
 *   RateLimitConfig config(50, 60, true, true); // 50 req/min por IP+user
 *   auto rateLimitMw = createRateLimitMiddleware(config);
 */
MiddlewareFunction createRateLimitMiddleware(
    int maxRequests,
    int windowSeconds,
    bool byIp = true,
    bool byUser = false
);

MiddlewareFunction createRateLimitMiddleware(const RateLimitConfig& config);

/**
 * @brief Storage global compartilhado
 * (Singleton para compartilhar entre todos os middlewares)
 */
Utils::RateLimitStorage& getRateLimitStorage();

/**
 * @brief Extrai IP do request
 * Verifica headers X-Forwarded-For, X-Real-IP antes de usar IP direto
 */
std::string extractClientIp(const Request& req);

} // namespace Core::Http

