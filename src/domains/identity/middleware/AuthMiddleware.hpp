#pragma once

#include "../../../core/http/Middleware.hpp"
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../services/JwtService.hpp"
#include <memory>
#include <string>
#include <functional>

namespace Domains::Identity::Middleware {

/**
 * @brief Middleware para autenticação JWT
 * 
 * Verifica se o request tem um token JWT válido no header Authorization.
 * Se válido, adiciona userId e role no request.
 * Se inválido, retorna 401 Unauthorized.
 * 
 * @example
 *   auto jwtService = std::make_shared<JwtService>("secret", 60);
 *   auto authMiddleware = std::make_shared<AuthMiddleware>(jwtService);
 *   router.use(authMiddleware);
 */
class AuthMiddleware : public Core::Http::Middleware {
public:
    explicit AuthMiddleware(std::shared_ptr<Services::JwtService> jwtService);
    
    void handle(
        Core::Http::Request& req, 
        Core::Http::Response& res,
        std::function<void()> next
    ) override;

private:
    std::shared_ptr<Services::JwtService> jwtService_;
    
    /**
     * @brief Extrai o token do header Authorization
     * @param authHeader Header "Authorization: Bearer <token>"
     * @return Token JWT ou string vazia se não encontrado
     */
    std::string extractToken(const std::string& authHeader) const;
};

/**
 * @brief Middleware para verificar role específica
 * 
 * Verifica se o usuário autenticado tem a role necessária.
 * Requer que AuthMiddleware seja executado antes.
 * 
 * @example
 *   auto roleMiddleware = std::make_shared<RoleMiddleware>("admin");
 *   router.use("/admin", roleMiddleware);
 */
class RoleMiddleware : public Core::Http::Middleware {
public:
    explicit RoleMiddleware(const std::string& requiredRole);
    
    void handle(
        Core::Http::Request& req, 
        Core::Http::Response& res,
        std::function<void()> next
    ) override;

private:
    std::string requiredRole_;
};

} // namespace Domains::Identity::Middleware

