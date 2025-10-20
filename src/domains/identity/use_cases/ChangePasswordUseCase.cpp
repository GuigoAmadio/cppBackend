#include "ChangePasswordUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Identity::UseCases {

ChangePasswordUseCase::ChangePasswordUseCase(
    std::shared_ptr<UserRepository> repository,
    std::shared_ptr<Services::BcryptService> bcryptService
) : repository_(repository), bcryptService_(bcryptService) {}

void ChangePasswordUseCase::execute(const ChangePasswordDto& dto) {
    // 1. Buscar usuário
    auto userOpt = repository_->findById(dto.userId);
    
    if (!userOpt.has_value()) {
        LOG_WARNING("ChangePassword failed: user not found - " + dto.userId);
        throw std::runtime_error("User not found");
    }
    
    User user = userOpt.value();
    
    // 2. Verificar autorização (usuário só pode trocar sua própria senha)
    if (dto.userId != dto.requestingUserId) {
        LOG_WARNING("ChangePassword failed: unauthorized attempt by " + dto.requestingUserId + 
                   " to change password of " + dto.userId);
        throw std::runtime_error("You can only change your own password");
    }
    
    // 3. Validar senha antiga
    if (!bcryptService_->verify(dto.oldPassword, user.getPasswordHash())) {
        LOG_WARNING("ChangePassword failed: invalid old password - " + dto.userId);
        throw std::runtime_error("Current password is incorrect");
    }
    
    // 4. Validar nova senha (será validado pelo Password value object)
    Password newPassword(dto.newPassword);
    
    // 5. Hash da nova senha
    std::string hashedPassword = bcryptService_->hash(dto.newPassword);
    Password hashedPasswordObj(hashedPassword, true); // true = já está hasheado
    
    // 6. Atualizar senha
    user.changePassword(hashedPasswordObj);
    
    // 7. Salvar
    repository_->update(user);
    
    LOG_INFO("Password changed successfully for user: " + user.getId());
    
    // TODO: Implementar invalidação de tokens (logout forçado)
    // Isso requerá um sistema de blacklist de tokens ou versão de senha no token
}

} // namespace Domains::Identity::UseCases

