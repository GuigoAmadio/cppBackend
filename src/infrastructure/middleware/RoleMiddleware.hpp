#pragma once

#include "../../core/http/Middleware.hpp"
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

namespace Core::Http {

/**
 * @brief Factory que cria um middleware de autorização por role
 * 
 * Verifica se o usuário tem uma das roles permitidas.
 * IMPORTANTE: Deve ser usado DEPOIS do AuthMiddleware!
 * 
 * @example
 *   // Apenas admins e owners podem acessar
 *   auto adminOnly = createRoleMiddleware({"admin", "owner"});
 *   
 *   router.post("/api/admin/action", 
 *       withAuth(withRole(adminOnly, [](const Request& req) {
 *           // ... handler
 *       })));
 * 
 * @param allowedRoles Vector com roles permitidas
 * @return MiddlewareFunction
 */
MiddlewareFunction createRoleMiddleware(const std::vector<std::string>& allowedRoles);

/**
 * @brief Verifica se role está na lista de permitidas
 */
bool hasRole(const std::string& userRole, const std::vector<std::string>& allowedRoles);

/**
 * @brief Helper para criar middleware que requer role de admin
 */
inline MiddlewareFunction requireAdmin() {
    return createRoleMiddleware({"admin", "owner"});
}

/**
 * @brief Helper para criar middleware que requer owner
 */
inline MiddlewareFunction requireOwner() {
    return createRoleMiddleware({"owner"});
}

} // namespace Core::Http

