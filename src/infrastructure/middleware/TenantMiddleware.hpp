#pragma once

#include "../../core/http/Middleware.hpp"
#include "../../domains/tenant_management/repositories/TenantRepository.hpp"
#include <memory>
#include <string>

namespace Core::Http {

/**
 * @brief Factory que cria um middleware para identificar e resolver tenant
 * 
 * Extrai o subdomain do header `Host` (ex: demo.localhost:8080 -> "demo")
 * e busca o tenant correspondente no banco de dados.
 * 
 * Adiciona informações do tenant ao request como custom data:
 * - "tenant_id": UUID do tenant
 * - "tenant_subdomain": Subdomain do tenant
 * - "tenant_name": Nome do tenant
 * 
 * @example
 *   auto tenantRepo = std::make_shared<TenantRepository>(pool);
 *   auto tenantMw = createTenantMiddleware(tenantRepo);
 *   // Usar tenantMw como função middleware
 */

/**
 * @brief Extrai subdomain do header Host
 */
std::string extractSubdomain(const std::string& host);

/**
 * @brief Cria middleware de tenant resolution
 */
MiddlewareFunction createTenantMiddleware(
    std::shared_ptr<Domains::TenantManagement::Repositories::TenantRepository> tenantRepo
);

} // namespace Core::Http

