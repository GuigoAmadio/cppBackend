#include "UpdateUserUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Identity::UseCases {

UpdateUserUseCase::UpdateUserUseCase(std::shared_ptr<UserRepository> repository)
    : repository_(repository) {}

User UpdateUserUseCase::execute(const UpdateUserDto& dto) {
    // 1. Buscar usuário a ser atualizado
    auto userOpt = repository_->findById(dto.userId);
    
    if (!userOpt.has_value()) {
        LOG_WARNING("UpdateUser failed: user not found - " + dto.userId);
        throw std::runtime_error("User not found");
    }
    
    User user = userOpt.value();
    
    // 2. Verificar autorização (usuário só pode atualizar seus próprios dados)
    // TODO: Adicionar verificação de admin role quando implementarmos RoleMiddleware
    if (dto.userId != dto.requestingUserId) {
        LOG_WARNING("UpdateUser failed: unauthorized attempt by " + dto.requestingUserId + 
                   " to update " + dto.userId);
        throw std::runtime_error("You can only update your own profile");
    }
    
    // 3. Atualizar nome se fornecido
    if (dto.name.has_value() && !dto.name.value().empty()) {
        user.changeName(dto.name.value());
        LOG_INFO("User name updated: " + dto.userId);
    }
    
    // 4. Atualizar email se fornecido
    if (dto.email.has_value() && !dto.email.value().empty()) {
        Email newEmail(dto.email.value());
        
        // Verificar se email já está em uso por outro usuário
        auto existingUserOpt = repository_->findByEmail(dto.email.value());
        if (existingUserOpt.has_value() && existingUserOpt.value().getId() != dto.userId) {
            LOG_WARNING("UpdateUser failed: email already in use - " + dto.email.value());
            throw std::runtime_error("Email already in use");
        }
        
        user.changeEmail(newEmail);
        LOG_INFO("User email updated: " + dto.userId + " (requires verification)");
    }
    
    // 5. Salvar alterações
    repository_->update(user);
    
    LOG_INFO("User updated successfully: " + user.getId());
    
    return user;
}

} // namespace Domains::Identity::UseCases

