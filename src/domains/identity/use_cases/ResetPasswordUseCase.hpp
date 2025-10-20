#pragma once

#include "../repositories/PasswordResetRepository.hpp"
#include "../repositories/UserRepository.hpp"
#include "../services/BcryptService.hpp"
#include <memory>
#include <string>

namespace Domains::Identity::UseCases {

struct ResetPasswordDto {
    std::string token;
    std::string newPassword;
};

class ResetPasswordUseCase {
public:
    ResetPasswordUseCase(
        std::shared_ptr<Repositories::PasswordResetRepository> passwordResetRepo,
        std::shared_ptr<Repositories::UserRepository> userRepo,
        std::shared_ptr<Services::BcryptService> bcryptService
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
    std::shared_ptr<Repositories::PasswordResetRepository> passwordResetRepo_;
    std::shared_ptr<Repositories::UserRepository> userRepo_;
    std::shared_ptr<Services::BcryptService> bcryptService_;
};

} // namespace Domains::Identity::UseCases

