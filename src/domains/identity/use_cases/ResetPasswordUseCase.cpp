#include "ResetPasswordUseCase.hpp"
#include "../value_objects/Password.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Identity::UseCases {

ResetPasswordUseCase::ResetPasswordUseCase(
    std::shared_ptr<Repositories::PasswordResetRepository> passwordResetRepo,
    std::shared_ptr<Repositories::UserRepository> userRepo,
    std::shared_ptr<Services::BcryptService> bcryptService
) : passwordResetRepo_(passwordResetRepo), userRepo_(userRepo), bcryptService_(bcryptService) {}

void ResetPasswordUseCase::execute(const ResetPasswordDto& dto) {
    // 1. Buscar token
    auto tokenOpt = passwordResetRepo_->findByToken(dto.token);
    if (!tokenOpt.has_value()) {
        throw std::runtime_error("Invalid password reset token");
    }
    
    auto token = tokenOpt.value();
    
    // 2. Verificar se token já foi usado
    if (token.usedAt.has_value()) {
        throw std::runtime_error("Password reset token already used");
    }
    
    // 3. Verificar se token expirou
    auto now = std::chrono::system_clock::now();
    if (now > token.expiresAt) {
        throw std::runtime_error("Password reset token expired");
    }
    
    // 4. Buscar usuário
    auto userOpt = userRepo_->findById(token.userId);
    if (!userOpt.has_value()) {
        throw std::runtime_error("User not found");
    }
    
    auto user = userOpt.value();
    
    // 5. Validar nova senha
    ValueObjects::Password newPassword(dto.newPassword, false);
    
    // 6. Hash da nova senha
    std::string hashedPassword = bcryptService_->hash(dto.newPassword);
    ValueObjects::Password hashedPasswordObj(hashedPassword, true);
    
    // 7. Atualizar senha do usuário
    user.changePassword(hashedPasswordObj);
    userRepo_->update(user);
    
    // 8. Marcar token como usado
    passwordResetRepo_->markAsUsed(dto.token);
    
    // TODO: Invalidar todos os refresh tokens (logout forçado em todos os dispositivos)
    
    LOG_INFO("Password reset successfully for user: " + user.getEmail().value());
}

} // namespace Domains::Identity::UseCases

