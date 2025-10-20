#pragma once

#include "../repositories/PasswordResetRepository.hpp"
#include "../repositories/UserRepository.hpp"
#include "../services/EmailService.hpp"
#include <memory>
#include <string>

namespace Domains::Identity::UseCases {

struct RequestPasswordResetDto {
    std::string email;
    std::string ipAddress;
    std::string userAgent;
};

class RequestPasswordResetUseCase {
public:
    RequestPasswordResetUseCase(
        std::shared_ptr<Repositories::PasswordResetRepository> passwordResetRepo,
        std::shared_ptr<Repositories::UserRepository> userRepo,
        std::shared_ptr<Services::EmailService> emailService
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
    std::shared_ptr<Repositories::PasswordResetRepository> passwordResetRepo_;
    std::shared_ptr<Repositories::UserRepository> userRepo_;
    std::shared_ptr<Services::EmailService> emailService_;
    
    std::string generateToken();
};

} // namespace Domains::Identity::UseCases

