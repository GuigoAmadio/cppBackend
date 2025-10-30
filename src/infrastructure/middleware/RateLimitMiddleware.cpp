#include "RateLimitMiddleware.hpp"
#include "../../core/utils/LoggerNew.hpp"
#include "../../core/json/Json.hpp"

namespace Core::Http {

// Singleton para storage global
Utils::RateLimitStorage& getRateLimitStorage() {
    static Utils::RateLimitStorage storage;
    return storage;
}

std::string extractClientIp(const Request& req) {
    // Tentar X-Forwarded-For primeiro (para proxies/load balancers)
    std::string xForwardedFor = req.getHeader("X-Forwarded-For");
    if (!xForwardedFor.empty()) {
        // Pega o primeiro IP da lista
        size_t commaPos = xForwardedFor.find(',');
        if (commaPos != std::string::npos) {
            return xForwardedFor.substr(0, commaPos);
        }
        return xForwardedFor;
    }
    
    // Tentar X-Real-IP
    std::string xRealIp = req.getHeader("X-Real-IP");
    if (!xRealIp.empty()) {
        return xRealIp;
    }
    
    // Fallback: usar algum identificador do request
    // Em produção, isso viria do socket connection
    return "unknown"; // TODO: Extrair IP real do socket
}

MiddlewareFunction createRateLimitMiddleware(
    int maxRequests,
    int windowSeconds,
    bool byIp,
    bool byUser
) {
    RateLimitConfig config(maxRequests, windowSeconds, byIp, byUser);
    return createRateLimitMiddleware(config);
}

MiddlewareFunction createRateLimitMiddleware(const RateLimitConfig& config) {
    auto& storage = getRateLimitStorage();
    
    return [config, &storage](Request& req, Response& res, NextFunction next) {
        std::string rateLimitKey;
        
        // Construir chave baseada na configuração
        if (config.byIp && config.byUser) {
            // Combinar IP + user_id
            std::string ip = extractClientIp(req);
            std::string userId = req.getCustomData("user_id");
            if (userId.empty()) {
                // Se não autenticado, usar apenas IP
                rateLimitKey = "ip:" + ip;
            } else {
                rateLimitKey = "ip:" + ip + ":user:" + userId;
            }
        } else if (config.byUser) {
            // Apenas por usuário autenticado
            std::string userId = req.getCustomData("user_id");
            if (userId.empty()) {
                // Se não autenticado, não aplicar rate limit
                LOG_DEBUG("RateLimit: skipping (not authenticated, user-based limit)");
                next();
                return;
            }
            rateLimitKey = "user:" + userId;
        } else {
            // Apenas por IP (padrão)
            std::string ip = extractClientIp(req);
            rateLimitKey = "ip:" + ip;
        }
        
        LOG_DEBUG("RateLimit: checking key=" + rateLimitKey + " limit=" + 
                 std::to_string(config.maxRequests) + "/" + std::to_string(config.windowSeconds) + "s");
        
        // Verificar limite
        if (!storage.checkLimit(rateLimitKey, config.maxRequests, config.windowSeconds)) {
            // Limite excedido!
            int resetTime = storage.getResetTime(rateLimitKey, config.windowSeconds);
            
            LOG_WARNING("RateLimit: limit exceeded for " + rateLimitKey + 
                       " (reset in " + std::to_string(resetTime) + "s)");
            
            auto json = Core::Json::makeObject();
            json->asObject()["status"] = Core::Json::makeString("error");
            
            std::string message = config.message.empty() 
                ? "Too many requests. Please try again in " + std::to_string(resetTime) + " seconds."
                : config.message;
            
            json->asObject()["message"] = Core::Json::makeString(message);
            json->asObject()["retryAfter"] = Core::Json::makeNumber(resetTime);
            
            // Headers padrão de rate limiting
            // res.setHeader("X-RateLimit-Limit", std::to_string(config.maxRequests));
            // res.setHeader("X-RateLimit-Remaining", "0");
            // res.setHeader("X-RateLimit-Reset", std::to_string(resetTime));
            // res.setHeader("Retry-After", std::to_string(resetTime));
            
            res = Response(StatusCode::TooManyRequests).json(*json);
            return;
        }
        
        // Registrar request
        storage.recordRequest(rateLimitKey);
        
        // Calcular remaining
        int remaining = storage.getRemainingRequests(rateLimitKey, config.maxRequests, config.windowSeconds);
        
        LOG_DEBUG("RateLimit: request allowed (" + std::to_string(remaining) + " remaining)");
        
        // Continuar para próximo middleware/handler
        next();
    };
}

} // namespace Core::Http

