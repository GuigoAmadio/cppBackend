#include "UpdateUserRoleUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::TenantManagement::UseCases {

UpdateUserRoleUseCase::UpdateUserRoleUseCase(std::shared_ptr<TenantRepository> repository)
    : repository_(repository) {}

void UpdateUserRoleUseCase::execute(const UpdateUserRoleDto& dto) {
    // 1. Verificar se tenant existe
    auto tenantOpt = repository_->findById(dto.tenantId);
    if (!tenantOpt.has_value()) {
        LOG_WARNING("UpdateUserRole failed: tenant not found - " + dto.tenantId);
        throw std::runtime_error("Tenant not found");
    }
    
    // 2. Verificar autorização (apenas quem pode gerenciar roles)
    if (!dto.requestingUserRole.canManageRoles()) {
        LOG_WARNING("UpdateUserRole failed: insufficient permissions - " + 
                   dto.requestingUserId + " (role: " + dto.requestingUserRole.toString() + ")");
        throw std::runtime_error("Only owners and admins can manage roles");
    }
    
    // 3. Verificar se não está tentando se auto-promover
    if (dto.userId == dto.requestingUserId) {
        LOG_WARNING("UpdateUserRole failed: self-promotion attempt - " + dto.requestingUserId);
        throw std::runtime_error("You cannot change your own role");
    }
    
    // 4. Verificar se não está promovendo acima da própria role
    if (!dto.requestingUserRole.canModifyRole(dto.newRole)) {
        LOG_WARNING("UpdateUserRole failed: attempting to assign higher role - " + 
                   dto.requestingUserId + " trying to assign " + dto.newRole.toString());
        throw std::runtime_error("You cannot assign a role higher than your own");
    }
    
    // 5. Verificar se usuário alvo tem acesso ao tenant
    if (!repository_->userHasAccessToTenant(dto.userId, dto.tenantId)) {
        LOG_WARNING("UpdateUserRole failed: user not in tenant - " + 
                   dto.userId + " @ " + dto.tenantId);
        throw std::runtime_error("User is not a member of this tenant");
    }
    
    // 6. Atualizar role
    repository_->updateUserRole(dto.userId, dto.tenantId, dto.newRole);
    
    LOG_INFO("User role updated: " + dto.userId + " -> " + dto.newRole.toString() + 
             " in tenant " + dto.tenantId + " by " + dto.requestingUserId);
}

} // namespace Domains::TenantManagement::UseCases

