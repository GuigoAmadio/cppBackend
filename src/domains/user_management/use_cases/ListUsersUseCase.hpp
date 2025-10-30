#pragma once

#include "../../user_management/entities/User.hpp"
#include "../../user_management/repositories/UserRepository.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Domains::UserManagement::UseCases {

// using namespace Entities; // Removed - use full namespace
using namespace Repositories;

/**
 * @brief DTO para listagem de usuários
 */
struct ListUsersDto {
    int limit = 50;           // Número de resultados por página
    int offset = 0;           // Offset para paginação
    std::string requestingUserRole; // Role do usuário fazendo a requisição
};

/**
 * @brief Resultado da listagem com paginação
 */
struct ListUsersResult {
    std::vector<User> users;
    int total;
    int limit;
    int offset;
};

/**
 * @brief Use Case: Listar usuários (com paginação)
 * 
 * Regras de negócio:
 * - Apenas admins podem listar usuários
 * - Suporta paginação
 * - Pode adicionar filtros (email, nome) no futuro
 */
class ListUsersUseCase {
public:
    explicit ListUsersUseCase(std::shared_ptr<UserRepository> repository);
    
    /**
     * @brief Executa o caso de uso
     * @throws std::runtime_error se não autorizado
     */
    ListUsersResult execute(const ListUsersDto& dto);

private:
    std::shared_ptr<UserRepository> repository_;
    
    bool isAdmin(const std::string& role) const;
};

} // namespace Domains::UserManagement::UseCases

