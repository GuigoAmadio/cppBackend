#include "AuthMiddleware.hpp"
#include "../../core/utils/LoggerNew.hpp"
#include "../../core/json/Json.hpp"

namespace Core::Http {

std::string extractBearerToken(const std::string& authHeader) {
    // Formato esperado: "Bearer <token>"
    if (authHeader.empty()) {
        return "";
    }
    
    // Verificar se começa com "Bearer "
    const std::string prefix = "Bearer ";
    if (authHeader.size() <= prefix.size()) {
        return "";
    }
    
    if (authHeader.substr(0, prefix.size()) != prefix) {
        return "";
    }
    
    // Extrair token (após "Bearer ")
    return authHeader.substr(prefix.size());
}

MiddlewareFunction createAuthMiddleware(
    std::shared_ptr<Domains::IAM::Services::JwtService> jwtService,
    bool optional
) {
    return [jwtService, optional](Request& req, Response& res, NextFunction next) {
        // 1. Extrair header Authorization
        std::string authHeader = req.getHeader("Authorization");
        
        if (authHeader.empty()) {
            authHeader = req.getHeader("authorization"); // Try lowercase
        }
        
        if (authHeader.empty()) {
            if (optional) {
                LOG_DEBUG("AuthMiddleware: No Authorization header (optional mode)");
                next();
                return;
            }
            
            LOG_WARNING("AuthMiddleware: No Authorization header");
            
            auto json = Core::Json::makeObject();
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString("Authorization header required");
            
            res = Response(StatusCode::Unauthorized).json(*json);
            return;
        }
        
        LOG_DEBUG("AuthMiddleware: Authorization header found");
        
        // 2. Extrair token
        std::string token = extractBearerToken(authHeader);
        
        if (token.empty()) {
            LOG_WARNING("AuthMiddleware: Invalid Authorization format (expected 'Bearer <token>')");
            
            auto json = Core::Json::makeObject();
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString("Invalid Authorization format (expected 'Bearer <token>')");
            
            res = Response(StatusCode::Unauthorized).json(*json);
            return;
        }
        
        LOG_DEBUG("AuthMiddleware: Token extracted successfully");
        
        // 3. Validar token
        try {
            auto claims = jwtService->validateToken(token);
            
            if (claims.empty()) {
                LOG_WARNING("AuthMiddleware: Token validation failed (invalid signature or expired)");
                
                auto json = Core::Json::makeObject();
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString("Invalid or expired token");
                
                res = Response(StatusCode::Unauthorized).json(*json);
                return;
            }
            
            // 4. Verificar se token expirou
            if (jwtService->isTokenExpired(token)) {
                LOG_WARNING("AuthMiddleware: Token expired");
                
                auto json = Core::Json::makeObject();
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString("Token expired");
                
                res = Response(StatusCode::Unauthorized).json(*json);
                return;
            }
            
            // 5. Extrair claims e adicionar ao request
            std::string userId = claims.count("sub") ? claims["sub"] : "";
            std::string email = claims.count("email") ? claims["email"] : "";
            std::string tenantId = claims.count("tenant_id") ? claims["tenant_id"] : "";
            std::string tenantSubdomain = claims.count("tenant_subdomain") ? claims["tenant_subdomain"] : "";
            std::string role = claims.count("role") ? claims["role"] : "";
            
            if (userId.empty()) {
                LOG_ERROR("AuthMiddleware: Token missing 'sub' (user_id) claim");
                
                auto json = Core::Json::makeObject();
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString("Invalid token structure");
                
                res = Response(StatusCode::Unauthorized).json(*json);
                return;
            }
            
            // Adicionar informações do usuário ao request
            req.setCustomData("user_id", userId);
            req.setCustomData("user_email", email);
            
            if (!tenantId.empty()) {
                req.setCustomData("user_tenant_id", tenantId);
            }
            
            if (!tenantSubdomain.empty()) {
                req.setCustomData("user_tenant_subdomain", tenantSubdomain);
            }
            
            if (!role.empty()) {
                req.setCustomData("user_role", role);
            }
            
            LOG_INFO("AuthMiddleware: User authenticated - user_id=" + userId + 
                     (tenantId.empty() ? "" : ", tenant_id=" + tenantId) +
                     (role.empty() ? "" : ", role=" + role));
            
            // 6. Continuar para próximo middleware/handler
            next();
            
        } catch (const std::exception& e) {
            LOG_ERROR("AuthMiddleware: Exception during token validation: " + std::string(e.what()));
            
            auto json = Core::Json::makeObject();
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString("Internal error validating token");
            
            res = Response(StatusCode::InternalServerError).json(*json);
            return;
        }
    };
}

} // namespace Core::Http

