#pragma once

#include "../../user_management/entities/User.hpp"
#include "../../user_management/repositories/UserRepository.hpp"
#include "../../audit/repositories/AuditLogRepository.hpp"
#include "../../tenant_management/repositories/TenantRepository.hpp"
#include "../value_objects/Email.hpp"
#include "../value_objects/Password.hpp"
#include "../../iam/services/BcryptService.hpp"
#include <memory>
#include <string>

namespace Domains::IAM::UseCases {

using Domains::UserManagement::Entities::User;
using namespace Domains::IAM::ValueObjects;

/**
 * @brief DTO para registro de usuário.
 * @note Role é atribuído por tenant na tabela user_tenants
 * @note Se tenant_subdomain for vazio, um novo tenant será criado automaticamente
 */
struct RegisterUserDto {
    std::string email;
    std::string password;
    std::string name;
    std::string tenant_subdomain; // Opcional: se vazio, cria novo tenant
};

/**
 * @brief Use Case: Registrar novo usuário.
 * 
 * Regras de negócio:
 * - Email deve ser único
 * - Senha deve atender critérios de segurança
 * - Nome deve ser válido
 * - Se tenant_subdomain fornecido: adiciona user ao tenant existente (role: member)
 * - Se tenant_subdomain NÃO fornecido: cria novo tenant automaticamente (role: admin)
 */
class RegisterUserUseCase {
public:
    explicit RegisterUserUseCase(
        std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> repository,
        std::shared_ptr<Domains::IAM::Services::BcryptService> bcryptService,
        std::shared_ptr<Domains::Audit::Repositories::AuditLogRepository> auditLogRepository,
        std::shared_ptr<Domains::TenantManagement::Repositories::TenantRepository> tenantRepository
    );
    
    /**
     * @brief Executa o caso de uso.
     * @throws std::invalid_argument se validação falhar
     * @throws std::runtime_error se email já existe ou tenant não encontrado
     */
    User execute(const RegisterUserDto& dto);

private:
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> repository_;
    std::shared_ptr<Domains::IAM::Services::BcryptService> bcryptService_;
    std::shared_ptr<Domains::Audit::Repositories::AuditLogRepository> auditLogRepository_;
    std::shared_ptr<Domains::TenantManagement::Repositories::TenantRepository> tenantRepository_;
    
    // Helper methods para criação de tenant
    std::string generateSlug(const std::string& input);
    std::string ensureUniqueSlug(const std::string& baseSlug);
    std::string generateTenantId(const std::string& slug);
};

} // namespace Domains::IAM::UseCases

