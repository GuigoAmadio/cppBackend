#pragma once

#include "../../tenant_management/repositories/TenantRepository.hpp"
#include <memory>
#include <string>

namespace Domains::TenantManagement::UseCases {

using namespace Repositories;

/**
 * @brief DTO para remoção de usuário do tenant
 */
struct RemoveUserFromTenantDto {
    std::string tenantId;
    std::string userId;              // User a ser removido
    std::string requestingUserId;    // User fazendo a remoção
    std::string requestingUserRole;  // Role do user fazendo a remoção
};

/**
 * @brief Use Case: Remover usuário de um tenant
 * 
 * Regras de negócio:
 * - Apenas owner/admin podem remover membros
 * - Owner não pode se auto-remover (deve transferir ownership primeiro)
 * - Não pode remover outro owner sem ser owner
 */
class RemoveUserFromTenantUseCase {
public:
    explicit RemoveUserFromTenantUseCase(std::shared_ptr<TenantRepository> repository);
    
    /**
     * @brief Executa o caso de uso
     * @throws std::runtime_error se não autorizado
     */
    void execute(const RemoveUserFromTenantDto& dto);

private:
    std::shared_ptr<TenantRepository> repository_;
    
    bool canManageMembers(const std::string& role) const;
};

} // namespace Domains::TenantManagement::UseCases

