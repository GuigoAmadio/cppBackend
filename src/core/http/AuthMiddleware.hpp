#pragma once

#include "Middleware.hpp"
#include "../../domains/identity/services/JwtService.hpp"
#include <memory>
#include <string>

namespace Core::Http {

/**
 * @brief Factory que cria um middleware de autenticação JWT
 * 
 * Valida o token JWT do header Authorization e adiciona informações
 * do usuário ao request:
 * - "user_id": ID do usuário
 * - "tenant_id": ID do tenant (se presente no token)
 * - "tenant_subdomain": Subdomain do tenant
 * - "role": Role do usuário no tenant
 * 
 * @example
 *   auto jwtService = std::make_shared<JwtService>(...);
 *   auto authMw = createAuthMiddleware(jwtService);
 *   
 *   // Usar em rota específica
 *   router.get("/api/protected", authMw, [](const Request& req) {
 *       std::string userId = req.getCustomData("user_id");
 *       // ...
 *   });
 *   
 *   // Ou globalmente via MiddlewareChain
 *   middlewares->use(authMw);
 * 
 * @param jwtService Shared pointer para JwtService
 * @param optional Se true, não bloqueia se token ausente (default: false)
 * @return MiddlewareFunction
 */
MiddlewareFunction createAuthMiddleware(
    std::shared_ptr<Domains::Identity::Services::JwtService> jwtService,
    bool optional = false
);

/**
 * @brief Extrai token do header Authorization
 * @param authHeader Header completo (ex: "Bearer eyJ...")
 * @return Token extraído ou string vazia
 */
std::string extractBearerToken(const std::string& authHeader);

} // namespace Core::Http

