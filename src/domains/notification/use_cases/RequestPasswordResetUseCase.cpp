#include "RequestPasswordResetUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <random>
#include <sstream>

namespace Domains::Notification::UseCases {

RequestPasswordResetUseCase::RequestPasswordResetUseCase(
    std::shared_ptr<Domains::Notification::Repositories::PasswordResetRepository> passwordResetRepo,
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepo,
    std::shared_ptr<Domains::Notification::Services::EmailService> emailService
) : passwordResetRepo_(passwordResetRepo), userRepo_(userRepo), emailService_(emailService) {}

std::string RequestPasswordResetUseCase::generateToken() {
    // Gerar UUID v4 (mesmo código de SendVerificationEmailUseCase)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4";
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }
    return ss.str();
}

void RequestPasswordResetUseCase::execute(const RequestPasswordResetDto& dto) {
    // 1. Buscar usuário por email
    auto userOpt = userRepo_->findByEmail(dto.email);
    
    // SEGURANÇA: Não revelar se o email existe ou não
    if (!userOpt.has_value()) {
        LOG_WARNING("Password reset requested for non-existent email: " + dto.email);
        // Não lançar erro! Retornar sucesso sempre.
        return;
    }
    
    auto user = userOpt.value();
    
    // 2. Verificar se usuário está ativo
    if (!user.isActive()) {
        LOG_WARNING("Password reset requested for inactive user: " + dto.email);
        return;
    }
    
    // 3. Deletar tokens antigos do usuário
    passwordResetRepo_->deleteByUserId(user.getId());
    
    // 4. Gerar novo token
    std::string token = generateToken();
    
    // 5. Calcular expiração (1 hora)
    auto expiresAt = std::chrono::system_clock::now() + std::chrono::hours(1);
    
    // 6. Salvar token no banco
    passwordResetRepo_->create(user.getId(), token, expiresAt, dto.ipAddress, dto.userAgent);
    
    // 7. Enviar email
    emailService_->sendPasswordResetEmail(user.getEmail().value(), user.getName(), token);
    
    LOG_INFO("Password reset email sent to: " + user.getEmail().value());
}

} // namespace Domains::Notification::UseCases

