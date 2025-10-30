#include "ListTenantMembersUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::TenantManagement::UseCases {

ListTenantMembersUseCase::ListTenantMembersUseCase(std::shared_ptr<TenantRepository> repository)
    : repository_(repository) {}

std::vector<TenantMember> ListTenantMembersUseCase::execute(const ListTenantMembersDto& dto) {
    // 1. Verificar se tenant existe
    auto tenantOpt = repository_->findById(dto.tenantId);
    if (!tenantOpt.has_value()) {
        LOG_WARNING("ListTenantMembers failed: tenant not found - " + dto.tenantId);
        throw std::runtime_error("Tenant not found");
    }
    
    // 2. Verificar se usuário tem acesso ao tenant
    if (!repository_->userHasAccessToTenant(dto.requestingUserId, dto.tenantId)) {
        LOG_WARNING("ListTenantMembers failed: user has no access to tenant - " + 
                   dto.requestingUserId + " @ " + dto.tenantId);
        throw std::runtime_error("You don't have access to this tenant");
    }
    
    // 3. Buscar membros do tenant
    auto rawMembers = repository_->getTenantMembers(dto.tenantId);
    
    // 4. Converter para TenantMember
    std::vector<TenantMember> members;
    for (const auto& m : rawMembers) {
        TenantMember member;
        member.userId = m.at("id");  // Note: getTenantUsers returns "id", not "user_id"
        member.email = m.at("email");
        member.name = m.at("name");
        member.role = m.at("role");
        member.isActive = (m.at("is_active") == "t" || m.at("is_active") == "true");
        members.push_back(member);
    }
    
    LOG_INFO("Tenant members listed: " + std::to_string(members.size()) + 
             " members in tenant " + dto.tenantId);
    
    return members;
}

} // namespace Domains::TenantManagement::UseCases

