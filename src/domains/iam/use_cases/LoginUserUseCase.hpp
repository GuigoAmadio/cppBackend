#pragma once

#include "../../user_management/entities/User.hpp"
#include "../../user_management/repositories/UserRepository.hpp"
#include "../../tenant_management/repositories/TenantRepository.hpp"
#include "../../audit/repositories/AuditLogRepository.hpp"
#include "../../iam/services/BcryptService.hpp"
#include "../../iam/services/JwtService.hpp"
#include <memory>
#include <string>

namespace Domains::IAM::UseCases {

using Domains::UserManagement::Entities::User;

/**
 * @brief DTO para login.
 * @note tenant_id e tenant_subdomain são extraídos do TenantMiddleware
 */
struct LoginUserDto {
    std::string email;
    std::string password;
    std::string tenant_id;           // Extraído do middleware
    std::string tenant_subdomain;    // Extraído do middleware
};

/**
 * @brief Resultado do login.
 */
struct LoginResult {
    User user;
    std::string token;
    std::string refreshToken;
};

/**
 * @brief Use Case: Login de usuário.
 * 
 * Regras de negócio:
 * - Usuário deve existir
 * - Senha deve estar correta
 * - Usuário deve estar ativo
 */
class LoginUserUseCase {
public:
    explicit LoginUserUseCase(
        std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> repository,
        std::shared_ptr<Domains::TenantManagement::Repositories::TenantRepository> tenantRepository,
        std::shared_ptr<Domains::IAM::Services::BcryptService> bcryptService,
        std::shared_ptr<Domains::IAM::Services::JwtService> jwtService,
        std::shared_ptr<Domains::Audit::Repositories::AuditLogRepository> auditLogRepository
    );
    
    /**
     * @brief Executa o caso de uso.
     * @throws std::runtime_error se credenciais inválidas
     */
    LoginResult execute(const LoginUserDto& dto);

private:
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> repository_;
    std::shared_ptr<Domains::TenantManagement::Repositories::TenantRepository> tenantRepository_;
    std::shared_ptr<Domains::IAM::Services::BcryptService> bcryptService_;
    std::shared_ptr<Domains::IAM::Services::JwtService> jwtService_;
    std::shared_ptr<Domains::Audit::Repositories::AuditLogRepository> auditLogRepository_;
};

} // namespace Domains::IAM::UseCases

