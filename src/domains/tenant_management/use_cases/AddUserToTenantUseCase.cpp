#include "AddUserToTenantUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::TenantManagement::UseCases {

AddUserToTenantUseCase::AddUserToTenantUseCase(
    std::shared_ptr<TenantRepository> tenantRepository,
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepository
) : tenantRepository_(tenantRepository), userRepository_(userRepository) {}

bool AddUserToTenantUseCase::canManageMembers(const std::string& role) const {
    return role == "owner" || role == "admin";
}

int AddUserToTenantUseCase::getRolePriority(const std::string& role) const {
    if (role == "owner") return 4;
    if (role == "admin") return 3;
    if (role == "user") return 2;
    if (role == "viewer") return 1;
    return 0;
}

void AddUserToTenantUseCase::execute(const AddUserToTenantDto& dto) {
    // 1. Verificar se tenant existe
    auto tenantOpt = tenantRepository_->findById(dto.tenantId);
    if (!tenantOpt.has_value()) {
        LOG_WARNING("AddUserToTenant failed: tenant not found - " + dto.tenantId);
        throw std::runtime_error("Tenant not found");
    }
    
    // 2. Verificar autorização (apenas owner/admin)
    if (!canManageMembers(dto.requestingUserRole)) {
        LOG_WARNING("AddUserToTenant failed: insufficient permissions - " + 
                   dto.requestingUserId + " (role: " + dto.requestingUserRole + ")");
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
    int requestingPriority = getRolePriority(dto.requestingUserRole);
    int newRolePriority = getRolePriority(dto.role);
    
    if (newRolePriority > requestingPriority) {
        LOG_WARNING("AddUserToTenant failed: attempting to assign higher role - " + 
                   dto.requestingUserId + " trying to assign " + dto.role);
        throw std::runtime_error("You cannot assign a role higher than your own");
    }
    
    // 6. Adicionar usuário ao tenant
    tenantRepository_->addUserToTenant(dto.userId, dto.tenantId, dto.role, dto.requestingUserId);
    
    LOG_INFO("User added to tenant: " + dto.userId + " added to tenant " + 
             dto.tenantId + " with role " + dto.role + " by " + dto.requestingUserId);
}

} // namespace Domains::TenantManagement::UseCases

