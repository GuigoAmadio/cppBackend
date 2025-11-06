#pragma once

#include "../../tenant_management/repositories/TenantRepository.hpp"
#include "../../tenant_management/value_objects/TenantUserRole.hpp"
#include "../../user_management/repositories/UserRepository.hpp"
#include <memory>
#include <string>

namespace Domains::TenantManagement::UseCases {

using namespace Repositories;

/**
 * @brief DTO para adicionar usuário ao tenant
 */
struct AddUserToTenantDto {
    std::string tenantId;
    std::string userId;              // User a ser adicionado (deve existir)
    ValueObjects::TenantUserRole role;  // Role inicial (default: "user")
    std::string requestingUserId;    // User fazendo a adição
    ValueObjects::TenantUserRole requestingUserRole;  // Role do user fazendo a adição
};

/**
 * @brief Use Case: Adicionar usuário existente a um tenant
 * 
 * Regras de negócio:
 * - Apenas owner/admin podem adicionar membros
 * - Usuário deve existir no sistema
 * - Usuário não pode já estar no tenant
 * - Não pode adicionar com role superior à própria
 */
class AddUserToTenantUseCase {
public:
    explicit AddUserToTenantUseCase(
        std::shared_ptr<TenantRepository> tenantRepository,
        std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepository
    );
    
    /**
     * @brief Executa o caso de uso
     * @throws std::runtime_error se não autorizado ou usuário não encontrado
     */
    void execute(const AddUserToTenantDto& dto);

private:
    std::shared_ptr<TenantRepository> tenantRepository_;
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepository_;
};

} // namespace Domains::TenantManagement::UseCases

