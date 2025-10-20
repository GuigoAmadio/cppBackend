#pragma once

#include "../entities/User.hpp"
#include "../repositories/UserRepository.hpp"
#include "../services/BcryptService.hpp"
#include "../value_objects/Password.hpp"
#include <memory>
#include <string>

namespace Domains::Identity::UseCases {

using namespace Entities;
using namespace Repositories;
using namespace ValueObjects;

/**
 * @brief DTO para troca de senha
 */
struct ChangePasswordDto {
    std::string userId;           // ID do usuário
    std::string requestingUserId; // ID do usuário fazendo a requisição
    std::string oldPassword;      // Senha atual (para validação)
    std::string newPassword;      // Nova senha
};

/**
 * @brief Use Case: Trocar senha do usuário
 * 
 * Regras de negócio:
 * - Deve validar senha antiga
 * - Nova senha deve ser forte
 * - Usuário só pode trocar sua própria senha
 * - Após trocar senha, deve invalidar tokens (logout forçado)
 */
class ChangePasswordUseCase {
public:
    explicit ChangePasswordUseCase(
        std::shared_ptr<UserRepository> repository,
        std::shared_ptr<Services::BcryptService> bcryptService
    );
    
    /**
     * @brief Executa o caso de uso
     * @throws std::runtime_error se senha antiga inválida ou não autorizado
     */
    void execute(const ChangePasswordDto& dto);

private:
    std::shared_ptr<UserRepository> repository_;
    std::shared_ptr<Services::BcryptService> bcryptService_;
};

} // namespace Domains::Identity::UseCases

