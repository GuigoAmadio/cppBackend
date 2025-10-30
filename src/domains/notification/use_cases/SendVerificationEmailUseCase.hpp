#pragma once

#include "../services/EmailVerificationRepository.hpp"
#include "../../user_management/repositories/UserRepository.hpp"
#include "../../notification/services/EmailService.hpp"
#include <memory>
#include <string>

namespace Domains::Notification::UseCases {

struct SendVerificationEmailDto {
    std::string userId;
};

class SendVerificationEmailUseCase {
public:
    SendVerificationEmailUseCase(
        std::shared_ptr<Domains::Notification::Repositories::EmailVerificationRepository> emailVerifRepo,
        std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepo,
        std::shared_ptr<Domains::Notification::Services::EmailService> emailService
    );
    
    /**
     * @brief Gera token e envia email de verificação
     * @param dto Dados do usuário
     * @throws std::runtime_error se usuário não existir ou email já verificado
     */
    void execute(const SendVerificationEmailDto& dto);

private:
    std::shared_ptr<Domains::Notification::Repositories::EmailVerificationRepository> emailVerifRepo_;
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepo_;
    std::shared_ptr<Domains::Notification::Services::EmailService> emailService_;
    
    std::string generateToken();
};

} // namespace Domains::Notification::UseCases

