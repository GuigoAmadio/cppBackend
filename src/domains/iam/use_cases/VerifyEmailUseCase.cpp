#include "VerifyEmailUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::IAM::UseCases {

VerifyEmailUseCase::VerifyEmailUseCase(
    std::shared_ptr<Domains::Notification::Repositories::EmailVerificationRepository> emailVerifRepo,
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepo,
    std::shared_ptr<Domains::Notification::Services::EmailService> emailService
) : emailVerifRepo_(emailVerifRepo), userRepo_(userRepo), emailService_(emailService) {}

void VerifyEmailUseCase::execute(const VerifyEmailDto& dto) {
    // 1. Buscar token
    auto tokenOpt = emailVerifRepo_->findByToken(dto.token);
    if (!tokenOpt.has_value()) {
        throw std::runtime_error("Invalid verification token");
    }
    
    auto token = tokenOpt.value();
    
    // 2. Verificar se token já foi usado
    if (token.usedAt.has_value()) {
        throw std::runtime_error("Verification token already used");
    }
    
    // 3. Verificar se token expirou
    auto now = std::chrono::system_clock::now();
    if (now > token.expiresAt) {
        throw std::runtime_error("Verification token expired");
    }
    
    // 4. Buscar usuário
    auto userOpt = userRepo_->findById(token.userId);
    if (!userOpt.has_value()) {
        throw std::runtime_error("User not found");
    }
    
    auto user = userOpt.value();
    
    // 5. Marcar email como verificado
    user.verifyEmail();
    userRepo_->update(user);
    
    // 6. Marcar token como usado
    emailVerifRepo_->markAsUsed(dto.token);
    
    // 7. Enviar email de boas-vindas
    emailService_->sendWelcomeEmail(user.getEmail().value(), user.getName());
    
    LOG_INFO("Email verified for user: " + user.getEmail().value());
}

} // namespace Domains::IAM::UseCases

