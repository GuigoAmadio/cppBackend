#include "DeleteUserUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Identity::UseCases {

DeleteUserUseCase::DeleteUserUseCase(std::shared_ptr<UserRepository> repository)
    : repository_(repository) {}

bool DeleteUserUseCase::isAdmin(const std::string& role) const {
    return role == "admin" || role == "owner";
}

void DeleteUserUseCase::execute(const DeleteUserDto& dto) {
    // 1. Buscar usuário a ser deletado
    auto userOpt = repository_->findById(dto.userId);
    
    if (!userOpt.has_value()) {
        LOG_WARNING("DeleteUser failed: user not found - " + dto.userId);
        throw std::runtime_error("User not found");
    }
    
    User user = userOpt.value();
    
    // 2. Verificar autorização
    // Usuário pode deletar sua própria conta OU ser admin
    bool isSelf = (dto.userId == dto.requestingUserId);
    bool isAdminUser = isAdmin(dto.requestingUserRole);
    
    if (!isSelf && !isAdminUser) {
        LOG_WARNING("DeleteUser failed: unauthorized attempt by " + dto.requestingUserId + 
                   " to delete " + dto.userId);
        throw std::runtime_error("You can only delete your own account or must be an admin");
    }
    
    // 3. Soft delete: marcar como inativo
    user.deactivate();
    
    // 4. Salvar
    repository_->update(user);
    
    LOG_INFO("User soft-deleted (deactivated): " + user.getId() + 
             " by " + (isSelf ? "self" : "admin " + dto.requestingUserId));
    
    // TODO: Remover de todos os tenants
    // TODO: Invalidar todos os tokens
    // TODO: Opcionalmente anonymizar dados pessoais (LGPD/GDPR)
}

} // namespace Domains::Identity::UseCases

