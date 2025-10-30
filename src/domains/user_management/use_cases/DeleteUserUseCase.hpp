#pragma once

#include "../../user_management/entities/User.hpp"
#include "../../user_management/repositories/UserRepository.hpp"
#include <memory>
#include <string>

namespace Domains::UserManagement::UseCases {

// using namespace Entities; // Removed - use full namespace
using namespace Repositories;

/**
 * @brief DTO para deleção de usuário
 */
struct DeleteUserDto {
    std::string userId;           // ID do usuário a ser deletado
    std::string requestingUserId; // ID do usuário fazendo a requisição
    std::string requestingUserRole; // Role do usuário fazendo a requisição
};

/**
 * @brief Use Case: Deletar usuário (soft delete)
 * 
 * Regras de negócio:
 * - Soft delete (marca como inativo ao invés de remover do banco)
 * - Usuário pode deletar sua própria conta
 * - Admin pode deletar qualquer conta
 * - Remove de todos os tenants
 * - Invalida todos os tokens
 */
class DeleteUserUseCase {
public:
    explicit DeleteUserUseCase(std::shared_ptr<UserRepository> repository);
    
    /**
     * @brief Executa o caso de uso
     * @throws std::runtime_error se não autorizado ou usuário não encontrado
     */
    void execute(const DeleteUserDto& dto);

private:
    std::shared_ptr<UserRepository> repository_;
    
    bool isAdmin(const std::string& role) const;
};

} // namespace Domains::UserManagement::UseCases

