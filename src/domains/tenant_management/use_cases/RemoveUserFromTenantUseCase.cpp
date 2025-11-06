#include "RemoveUserFromTenantUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::TenantManagement::UseCases {

RemoveUserFromTenantUseCase::RemoveUserFromTenantUseCase(std::shared_ptr<TenantRepository> repository)
    : repository_(repository) {}

void RemoveUserFromTenantUseCase::execute(const RemoveUserFromTenantDto& dto) {
    // 1. Verificar se tenant existe
    auto tenantOpt = repository_->findById(dto.tenantId);
    if (!tenantOpt.has_value()) {
        LOG_WARNING("RemoveUserFromTenant failed: tenant not found - " + dto.tenantId);
        throw std::runtime_error("Tenant not found");
    }
    
    // 2. Verificar autorização (apenas quem pode gerenciar membros)
    if (!dto.requestingUserRole.canManageMembers()) {
        LOG_WARNING("RemoveUserFromTenant failed: insufficient permissions - " + 
                   dto.requestingUserId + " (role: " + dto.requestingUserRole.toString() + ")");
        throw std::runtime_error("Only owners and admins can remove members");
    }
    
    // 3. Verificar se usuário a ser removido existe no tenant
    auto targetRoleOpt = repository_->getUserRoleInTenant(dto.userId, dto.tenantId);
    if (!targetRoleOpt.has_value()) {
        LOG_WARNING("RemoveUserFromTenant failed: user not in tenant - " + 
                   dto.userId + " @ " + dto.tenantId);
        throw std::runtime_error("User is not a member of this tenant");
    }
    
    auto targetRole = targetRoleOpt.value();
    
    // 4. Owner não pode se auto-remover
    if (dto.userId == dto.requestingUserId && dto.requestingUserRole.isOwner()) {
        LOG_WARNING("RemoveUserFromTenant failed: owner self-removal attempt - " + dto.requestingUserId);
        throw std::runtime_error("Owner cannot remove themselves. Transfer ownership first.");
    }
    
    // 5. Apenas owner pode remover outro owner
    if (targetRole.isOwner() && !dto.requestingUserRole.isOwner()) {
        LOG_WARNING("RemoveUserFromTenant failed: non-owner attempting to remove owner - " + 
                   dto.requestingUserId);
        throw std::runtime_error("Only owners can remove other owners");
    }
    
    // 6. Remover usuário
    repository_->removeUserFromTenant(dto.userId, dto.tenantId);
    
    LOG_INFO("User removed from tenant: " + dto.userId + " removed from tenant " + 
             dto.tenantId + " by " + dto.requestingUserId);
}

} // namespace Domains::TenantManagement::UseCases

