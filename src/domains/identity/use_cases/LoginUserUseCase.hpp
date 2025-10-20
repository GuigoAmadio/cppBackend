#pragma once

#include "../entities/User.hpp"
#include "../repositories/UserRepository.hpp"
#include "../repositories/TenantRepository.hpp"
#include "../services/BcryptService.hpp"
#include "../services/JwtService.hpp"
#include <memory>
#include <string>

namespace Domains::Identity::UseCases {

using namespace Entities;
using namespace Repositories;

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
        std::shared_ptr<UserRepository> repository,
        std::shared_ptr<TenantRepository> tenantRepository,
        std::shared_ptr<Services::BcryptService> bcryptService,
        std::shared_ptr<Services::JwtService> jwtService
    );
    
    /**
     * @brief Executa o caso de uso.
     * @throws std::runtime_error se credenciais inválidas
     */
    LoginResult execute(const LoginUserDto& dto);

private:
    std::shared_ptr<UserRepository> repository_;
    std::shared_ptr<TenantRepository> tenantRepository_;
    std::shared_ptr<Services::BcryptService> bcryptService_;
    std::shared_ptr<Services::JwtService> jwtService_;
};

} // namespace Domains::Identity::UseCases

