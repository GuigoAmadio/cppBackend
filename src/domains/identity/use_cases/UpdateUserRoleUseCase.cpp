#include "UpdateUserRoleUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Identity::UseCases {

UpdateUserRoleUseCase::UpdateUserRoleUseCase(std::shared_ptr<TenantRepository> repository)
    : repository_(repository) {}

bool UpdateUserRoleUseCase::canManageRoles(const std::string& role) const {
    return role == "owner" || role == "admin";
}

int UpdateUserRoleUseCase::getRolePriority(const std::string& role) const {
    if (role == "owner") return 4;
    if (role == "admin") return 3;
    if (role == "user") return 2;
    if (role == "viewer") return 1;
    return 0;
}

void UpdateUserRoleUseCase::execute(const UpdateUserRoleDto& dto) {
    // 1. Verificar se tenant existe
    auto tenantOpt = repository_->findById(dto.tenantId);
    if (!tenantOpt.has_value()) {
        LOG_WARNING("UpdateUserRole failed: tenant not found - " + dto.tenantId);
        throw std::runtime_error("Tenant not found");
    }
    
    // 2. Verificar autorização (apenas owner/admin)
    if (!canManageRoles(dto.requestingUserRole)) {
        LOG_WARNING("UpdateUserRole failed: insufficient permissions - " + 
                   dto.requestingUserId + " (role: " + dto.requestingUserRole + ")");
        throw std::runtime_error("Only owners and admins can manage roles");
    }
    
    // 3. Verificar se não está tentando se auto-promover
    if (dto.userId == dto.requestingUserId) {
        LOG_WARNING("UpdateUserRole failed: self-promotion attempt - " + dto.requestingUserId);
        throw std::runtime_error("You cannot change your own role");
    }
    
    // 4. Verificar se não está promovendo acima da própria role
    int requestingPriority = getRolePriority(dto.requestingUserRole);
    int newRolePriority = getRolePriority(dto.newRole);
    
    if (newRolePriority > requestingPriority) {
        LOG_WARNING("UpdateUserRole failed: attempting to assign higher role - " + 
                   dto.requestingUserId + " trying to assign " + dto.newRole);
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
    
    LOG_INFO("User role updated: " + dto.userId + " -> " + dto.newRole + 
             " in tenant " + dto.tenantId + " by " + dto.requestingUserId);
}

} // namespace Domains::Identity::UseCases

