#pragma once

#include "../repositories/EmailVerificationRepository.hpp"
#include "../repositories/UserRepository.hpp"
#include "../services/EmailService.hpp"
#include <memory>
#include <string>

namespace Domains::Identity::UseCases {

struct SendVerificationEmailDto {
    std::string userId;
};

class SendVerificationEmailUseCase {
public:
    SendVerificationEmailUseCase(
        std::shared_ptr<Repositories::EmailVerificationRepository> emailVerifRepo,
        std::shared_ptr<Repositories::UserRepository> userRepo,
        std::shared_ptr<Services::EmailService> emailService
    );
    
    /**
     * @brief Gera token e envia email de verificação
     * @param dto Dados do usuário
     * @throws std::runtime_error se usuário não existir ou email já verificado
     */
    void execute(const SendVerificationEmailDto& dto);

private:
    std::shared_ptr<Repositories::EmailVerificationRepository> emailVerifRepo_;
    std::shared_ptr<Repositories::UserRepository> userRepo_;
    std::shared_ptr<Services::EmailService> emailService_;
    
    std::string generateToken();
};

} // namespace Domains::Identity::UseCases

