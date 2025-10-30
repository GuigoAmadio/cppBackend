#include "ListUsersUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::UserManagement::UseCases {

ListUsersUseCase::ListUsersUseCase(std::shared_ptr<UserRepository> repository)
    : repository_(repository) {}

bool ListUsersUseCase::isAdmin(const std::string& role) const {
    return role == "admin" || role == "owner";
}

ListUsersResult ListUsersUseCase::execute(const ListUsersDto& dto) {
    // 1. Verificar autorização (apenas admins)
    if (!isAdmin(dto.requestingUserRole)) {
        LOG_WARNING("ListUsers failed: unauthorized role - " + dto.requestingUserRole);
        throw std::runtime_error("Only admins can list users");
    }
    
    // 2. Buscar usuários com paginação
    std::vector<User> users = repository_->findAll(dto.limit, dto.offset);
    
    // 3. Obter total de usuários (para paginação)
    // TODO: Implementar método countAll() no UserRepository
    int total = users.size(); // Temporário: retorna apenas o tamanho da página atual
    
    LOG_INFO("Users listed: " + std::to_string(users.size()) + " results (limit: " + 
             std::to_string(dto.limit) + ", offset: " + std::to_string(dto.offset) + ")");
    
    return ListUsersResult{
        .users = users,
        .total = total,
        .limit = dto.limit,
        .offset = dto.offset
    };
}

} // namespace Domains::UserManagement::UseCases

