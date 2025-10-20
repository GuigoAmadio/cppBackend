#pragma once

#include "../repositories/EmailVerificationRepository.hpp"
#include "../repositories/UserRepository.hpp"
#include "../services/EmailService.hpp"
#include <memory>
#include <string>

namespace Domains::Identity::UseCases {

struct VerifyEmailDto {
    std::string token;
};

class VerifyEmailUseCase {
public:
    VerifyEmailUseCase(
        std::shared_ptr<Repositories::EmailVerificationRepository> emailVerifRepo,
        std::shared_ptr<Repositories::UserRepository> userRepo,
        std::shared_ptr<Services::EmailService> emailService
    );
    
    /**
     * @brief Valida token e marca email como verificado
     * @param dto Token de verificação
     * @throws std::runtime_error se token inválido, expirado ou já usado
     */
    void execute(const VerifyEmailDto& dto);

private:
    std::shared_ptr<Repositories::EmailVerificationRepository> emailVerifRepo_;
    std::shared_ptr<Repositories::UserRepository> userRepo_;
    std::shared_ptr<Services::EmailService> emailService_;
};

} // namespace Domains::Identity::UseCases

