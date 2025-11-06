#include "AddUserToTenantUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::TenantManagement::UseCases {

AddUserToTenantUseCase::AddUserToTenantUseCase(
    std::shared_ptr<TenantRepository> tenantRepository,
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepository
) : tenantRepository_(tenantRepository), userRepository_(userRepository) {}

void AddUserToTenantUseCase::execute(const AddUserToTenantDto& dto) {
    // 1. Verificar se tenant existe
    auto tenantOpt = tenantRepository_->findById(dto.tenantId);
    if (!tenantOpt.has_value()) {
        LOG_WARNING("AddUserToTenant failed: tenant not found - " + dto.tenantId);
        throw std::runtime_error("Tenant not found");
    }
    
    // 2. Verificar autorização (apenas quem pode gerenciar membros)
    if (!dto.requestingUserRole.canManageMembers()) {
        LOG_WARNING("AddUserToTenant failed: insufficient permissions - " + 
                   dto.requestingUserId + " (role: " + dto.requestingUserRole.toString() + ")");
        throw std::runtime_error("Only owners and admins can add members");
    }
    
    // 3. Verificar se usuário a ser adicionado existe
    auto userOpt = userRepository_->findById(dto.userId);
    if (!userOpt.has_value()) {
        LOG_WARNING("AddUserToTenant failed: user not found - " + dto.userId);
        throw std::runtime_error("User not found");
    }
    
    // 4. Verificar se usuário já não está no tenant
    if (tenantRepository_->userHasAccessToTenant(dto.userId, dto.tenantId)) {
        LOG_WARNING("AddUserToTenant failed: user already in tenant - " + 
                   dto.userId + " @ " + dto.tenantId);
        throw std::runtime_error("User is already a member of this tenant");
    }
    
    // 5. Verificar se não está tentando adicionar com role superior à própria
    if (!dto.requestingUserRole.canModifyRole(dto.role)) {
        LOG_WARNING("AddUserToTenant failed: attempting to assign higher role - " + 
                   dto.requestingUserId + " trying to assign " + dto.role.toString());
        throw std::runtime_error("You cannot assign a role higher than your own");
    }
    
    // 6. Adicionar usuário ao tenant
    tenantRepository_->addUserToTenant(dto.userId, dto.tenantId, dto.role, dto.requestingUserId);
    
    LOG_INFO("User added to tenant: " + dto.userId + " added to tenant " + 
             dto.tenantId + " with role " + dto.role.toString() + " by " + dto.requestingUserId);
}

} // namespace Domains::TenantManagement::UseCases

