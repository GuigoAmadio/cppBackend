#pragma once

#include "../../tenant_management/repositories/TenantRepository.hpp"
#include "../../tenant_management/value_objects/TenantUserRole.hpp"
#include <memory>
#include <string>

namespace Domains::TenantManagement::UseCases {

using namespace Repositories;

/**
 * @brief DTO para atualização de role
 */
struct UpdateUserRoleDto {
    std::string tenantId;
    std::string userId;              // User cuja role será alterada
    ValueObjects::TenantUserRole newRole;  // Nova role
    std::string requestingUserId;    // User fazendo a alteração
    ValueObjects::TenantUserRole requestingUserRole;  // Role do user fazendo a alteração
};

/**
 * @brief Use Case: Atualizar role de um usuário em um tenant
 * 
 * Regras de negócio:
 * - Apenas owner/admin podem alterar roles
 * - Não pode se auto-promover
 * - Não pode promover acima da própria role
 */
class UpdateUserRoleUseCase {
public:
    explicit UpdateUserRoleUseCase(std::shared_ptr<TenantRepository> repository);
    
    /**
     * @brief Executa o caso de uso
     * @throws std::runtime_error se não autorizado
     */
    void execute(const UpdateUserRoleDto& dto);

private:
    std::shared_ptr<TenantRepository> repository_;
};

} // namespace Domains::TenantManagement::UseCases

