#pragma once

#include "../services/PasswordResetRepository.hpp"
#include "../../user_management/repositories/UserRepository.hpp"
#include "../../notification/services/EmailService.hpp"
#include <memory>
#include <string>

namespace Domains::Notification::UseCases {

struct RequestPasswordResetDto {
    std::string email;
    std::string ipAddress;
    std::string userAgent;
};

class RequestPasswordResetUseCase {
public:
    RequestPasswordResetUseCase(
        std::shared_ptr<Domains::Notification::Repositories::PasswordResetRepository> passwordResetRepo,
        std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepo,
        std::shared_ptr<Domains::Notification::Services::EmailService> emailService
    );
    
    /**
     * @brief Solicita reset de senha
     * 
     * Gera token, salva no banco e envia email.
     * NÃO deve retornar erro se email não existir (segurança).
     * 
     * @param dto Email do usuário
     */
    void execute(const RequestPasswordResetDto& dto);

private:
    std::shared_ptr<Domains::Notification::Repositories::PasswordResetRepository> passwordResetRepo_;
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepo_;
    std::shared_ptr<Domains::Notification::Services::EmailService> emailService_;
    
    std::string generateToken();
};

} // namespace Domains::Notification::UseCases

