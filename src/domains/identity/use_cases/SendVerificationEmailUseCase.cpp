#include "SendVerificationEmailUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <random>
#include <sstream>
#include <iomanip>

namespace Domains::Identity::UseCases {

SendVerificationEmailUseCase::SendVerificationEmailUseCase(
    std::shared_ptr<Repositories::EmailVerificationRepository> emailVerifRepo,
    std::shared_ptr<Repositories::UserRepository> userRepo,
    std::shared_ptr<Services::EmailService> emailService
) : emailVerifRepo_(emailVerifRepo), userRepo_(userRepo), emailService_(emailService) {}

std::string SendVerificationEmailUseCase::generateToken() {
    // Gerar UUID v4
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

void SendVerificationEmailUseCase::execute(const SendVerificationEmailDto& dto) {
    // 1. Buscar usuário
    auto userOpt = userRepo_->findById(dto.userId);
    if (!userOpt.has_value()) {
        throw std::runtime_error("User not found");
    }
    
    auto user = userOpt.value();
    
    // 2. Verificar se email já foi verificado
    if (user.isEmailVerified()) {
        throw std::runtime_error("Email already verified");
    }
    
    // 3. Deletar tokens antigos do usuário
    emailVerifRepo_->deleteByUserId(dto.userId);
    
    // 4. Gerar novo token
    std::string token = generateToken();
    
    // 5. Calcular expiração (24 horas)
    auto expiresAt = std::chrono::system_clock::now() + std::chrono::hours(24);
    
    // 6. Salvar token no banco
    emailVerifRepo_->create(dto.userId, token, expiresAt);
    
    // 7. Enviar email
    emailService_->sendVerificationEmail(user.getEmail().value(), user.getName(), token);
    
    LOG_INFO("Verification email sent to: " + user.getEmail().value());
}

} // namespace Domains::Identity::UseCases

