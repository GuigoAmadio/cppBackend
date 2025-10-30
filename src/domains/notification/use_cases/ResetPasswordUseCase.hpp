#pragma once

#include "../services/PasswordResetRepository.hpp"
#include "../../user_management/repositories/UserRepository.hpp"
#include "../../iam/services/BcryptService.hpp"
#include <memory>
#include <string>

namespace Domains::Notification::UseCases {

struct ResetPasswordDto {
    std::string token;
    std::string newPassword;
};

class ResetPasswordUseCase {
public:
    ResetPasswordUseCase(
        std::shared_ptr<Domains::Notification::Repositories::PasswordResetRepository> passwordResetRepo,
        std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepo,
        std::shared_ptr<Domains::IAM::Services::BcryptService> bcryptService
    );
    
    /**
     * @brief Reseta senha do usuário
     * 
     * Valida token, atualiza senha e marca token como usado.
     * 
     * @param dto Token e nova senha
     * @throws std::runtime_error se token inválido/expirado/usado
     */
    void execute(const ResetPasswordDto& dto);

private:
    std::shared_ptr<Domains::Notification::Repositories::PasswordResetRepository> passwordResetRepo_;
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepo_;
    std::shared_ptr<Domains::IAM::Services::BcryptService> bcryptService_;
};

} // namespace Domains::Notification::UseCases

