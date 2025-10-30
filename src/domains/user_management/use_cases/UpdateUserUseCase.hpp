#pragma once

#include "../../user_management/entities/User.hpp"
#include "../../user_management/repositories/UserRepository.hpp"
#include "../../iam/value_objects/Email.hpp"
#include <memory>
#include <string>
#include <optional>

namespace Domains::UserManagement::UseCases {

// using namespace Entities; // Removed - use full namespace
using namespace Repositories;
using namespace Domains::IAM::ValueObjects;

/**
 * @brief DTO para atualização de usuário
 */
struct UpdateUserDto {
    std::string userId;           // ID do usuário a ser atualizado
    std::string requestingUserId; // ID do usuário fazendo a requisição
    std::optional<std::string> name;  // Novo nome (opcional)
    std::optional<std::string> email; // Novo email (opcional)
};

/**
 * @brief Use Case: Atualizar dados do usuário
 * 
 * Regras de negócio:
 * - Usuário só pode atualizar seus próprios dados (ou admin)
 * - Se trocar email, requer nova verificação
 * - Email deve ser único
 */
class UpdateUserUseCase {
public:
    explicit UpdateUserUseCase(std::shared_ptr<UserRepository> repository);
    
    /**
     * @brief Executa o caso de uso
     * @throws std::runtime_error se usuário não encontrado ou não autorizado
     */
    User execute(const UpdateUserDto& dto);

private:
    std::shared_ptr<UserRepository> repository_;
};

} // namespace Domains::UserManagement::UseCases

