#pragma once

#include "../entities/User.hpp"
#include "../repositories/UserRepository.hpp"
#include "../value_objects/Email.hpp"
#include "../value_objects/Password.hpp"
#include "../services/BcryptService.hpp"
#include <memory>
#include <string>

namespace Domains::Identity::UseCases {

using namespace Entities;
using namespace Repositories;
using namespace ValueObjects;

/**
 * @brief DTO para registro de usuário.
 * @note Role é atribuído por tenant na tabela user_tenants
 */
struct RegisterUserDto {
    std::string email;
    std::string password;
    std::string name;
};

/**
 * @brief Use Case: Registrar novo usuário.
 * 
 * Regras de negócio:
 * - Email deve ser único
 * - Senha deve atender critérios de segurança
 * - Nome deve ser válido
 */
class RegisterUserUseCase {
public:
    explicit RegisterUserUseCase(
        std::shared_ptr<UserRepository> repository,
        std::shared_ptr<Services::BcryptService> bcryptService
    );
    
    /**
     * @brief Executa o caso de uso.
     * @throws std::invalid_argument se validação falhar
     * @throws std::runtime_error se email já existe
     */
    User execute(const RegisterUserDto& dto);

private:
    std::shared_ptr<UserRepository> repository_;
    std::shared_ptr<Services::BcryptService> bcryptService_;
};

} // namespace Domains::Identity::UseCases

