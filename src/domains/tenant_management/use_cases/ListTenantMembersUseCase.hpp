#pragma once

#include "../../tenant_management/repositories/TenantRepository.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Domains::TenantManagement::UseCases {

using namespace Repositories;

/**
 * @brief Informações de um membro do tenant
 */
struct TenantMember {
    std::string userId;
    std::string email;
    std::string name;
    std::string role;
    bool isActive;
};

/**
 * @brief DTO para listagem de membros
 */
struct ListTenantMembersDto {
    std::string tenantId;
    std::string requestingUserId;
    std::string requestingUserRole;
};

/**
 * @brief Use Case: Listar membros de um tenant
 * 
 * Regras de negócio:
 * - Apenas membros do tenant podem ver a lista
 * - Retorna informações básicas + role no tenant
 */
class ListTenantMembersUseCase {
public:
    explicit ListTenantMembersUseCase(std::shared_ptr<TenantRepository> repository);
    
    /**
     * @brief Executa o caso de uso
     * @throws std::runtime_error se não autorizado ou tenant não encontrado
     */
    std::vector<TenantMember> execute(const ListTenantMembersDto& dto);

private:
    std::shared_ptr<TenantRepository> repository_;
};

} // namespace Domains::TenantManagement::UseCases

