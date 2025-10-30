#pragma once

#include "../../notification/services/EmailVerificationRepository.hpp"
#include "../../user_management/repositories/UserRepository.hpp"
#include "../../notification/services/EmailService.hpp"
#include <memory>
#include <string>

namespace Domains::IAM::UseCases {

struct VerifyEmailDto {
    std::string token;
};

class VerifyEmailUseCase {
public:
    VerifyEmailUseCase(
        std::shared_ptr<Domains::Notification::Repositories::EmailVerificationRepository> emailVerifRepo,
        std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepo,
        std::shared_ptr<Domains::Notification::Services::EmailService> emailService
    );
    
    /**
     * @brief Valida token e marca email como verificado
     * @param dto Token de verificação
     * @throws std::runtime_error se token inválido, expirado ou já usado
     */
    void execute(const VerifyEmailDto& dto);

private:
    std::shared_ptr<Domains::Notification::Repositories::EmailVerificationRepository> emailVerifRepo_;
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepo_;
    std::shared_ptr<Domains::Notification::Services::EmailService> emailService_;
};

} // namespace Domains::IAM::UseCases

