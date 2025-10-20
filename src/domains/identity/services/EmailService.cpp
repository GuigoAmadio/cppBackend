#include "EmailService.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Identity::Services {

std::string MockEmailService::getVerificationUrl(const std::string& token) const {
    return "http://localhost:3000/verify-email?token=" + token;
}

std::string MockEmailService::getPasswordResetUrl(const std::string& token) const {
    return "http://localhost:3000/reset-password?token=" + token;
}

void MockEmailService::sendVerificationEmail(
    const std::string& email,
    const std::string& name,
    const std::string& token
) {
    LOG_INFO("==========================================");
    LOG_INFO("📧 [MOCK EMAIL] Verification Email");
    LOG_INFO("==========================================");
    LOG_INFO("To: " + email);
    LOG_INFO("Subject: Verify your email address");
    LOG_INFO("");
    LOG_INFO("Hi " + name + ",");
    LOG_INFO("");
    LOG_INFO("Thank you for registering! Please verify your email by clicking the link below:");
    LOG_INFO("");
    LOG_INFO(getVerificationUrl(token));
    LOG_INFO("");
    LOG_INFO("This link will expire in 24 hours.");
    LOG_INFO("");
    LOG_INFO("If you didn't create this account, please ignore this email.");
    LOG_INFO("==========================================");
}

void MockEmailService::sendPasswordResetEmail(
    const std::string& email,
    const std::string& name,
    const std::string& token
) {
    LOG_INFO("==========================================");
    LOG_INFO("🔑 [MOCK EMAIL] Password Reset");
    LOG_INFO("==========================================");
    LOG_INFO("To: " + email);
    LOG_INFO("Subject: Reset your password");
    LOG_INFO("");
    LOG_INFO("Hi " + name + ",");
    LOG_INFO("");
    LOG_INFO("You requested to reset your password. Click the link below:");
    LOG_INFO("");
    LOG_INFO(getPasswordResetUrl(token));
    LOG_INFO("");
    LOG_INFO("This link will expire in 1 hour.");
    LOG_INFO("");
    LOG_INFO("If you didn't request this, please ignore this email.");
    LOG_INFO("==========================================");
}

void MockEmailService::sendWelcomeEmail(
    const std::string& email,
    const std::string& name
) {
    LOG_INFO("==========================================");
    LOG_INFO("👋 [MOCK EMAIL] Welcome!");
    LOG_INFO("==========================================");
    LOG_INFO("To: " + email);
    LOG_INFO("Subject: Welcome to MoneyMaker!");
    LOG_INFO("");
    LOG_INFO("Hi " + name + ",");
    LOG_INFO("");
    LOG_INFO("Welcome to MoneyMaker! Your email has been verified.");
    LOG_INFO("");
    LOG_INFO("You can now start using all features of the platform.");
    LOG_INFO("");
    LOG_INFO("If you have any questions, feel free to contact us.");
    LOG_INFO("==========================================");
}

} // namespace Domains::Identity::Services

