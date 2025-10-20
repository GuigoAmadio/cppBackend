#pragma once

#include <string>
#include <memory>

namespace Domains::Identity::Services {

/**
 * @brief Interface abstrata para serviço de email
 * 
 * Permite trocar implementação facilmente:
 * - MockEmailService (para desenvolvimento/testes)
 * - SmtpEmailService (para produção)
 */
class EmailService {
public:
    virtual ~EmailService() = default;
    
    /**
     * @brief Envia email de verificação
     * @param email Email do destinatário
     * @param name Nome do usuário
     * @param token Token de verificação (UUID)
     */
    virtual void sendVerificationEmail(
        const std::string& email,
        const std::string& name,
        const std::string& token
    ) = 0;
    
    /**
     * @brief Envia email de reset de senha
     * @param email Email do destinatário
     * @param name Nome do usuário
     * @param token Token de reset (UUID)
     */
    virtual void sendPasswordResetEmail(
        const std::string& email,
        const std::string& name,
        const std::string& token
    ) = 0;
    
    /**
     * @brief Envia email de boas-vindas
     * @param email Email do destinatário
     * @param name Nome do usuário
     */
    virtual void sendWelcomeEmail(
        const std::string& email,
        const std::string& name
    ) = 0;
};

/**
 * @brief Implementação MOCK para desenvolvimento
 * 
 * Loga os emails no console ao invés de enviar.
 * Ideal para desenvolvimento e testes.
 */
class MockEmailService : public EmailService {
public:
    void sendVerificationEmail(
        const std::string& email,
        const std::string& name,
        const std::string& token
    ) override;
    
    void sendPasswordResetEmail(
        const std::string& email,
        const std::string& name,
        const std::string& token
    ) override;
    
    void sendWelcomeEmail(
        const std::string& email,
        const std::string& name
    ) override;

private:
    std::string getVerificationUrl(const std::string& token) const;
    std::string getPasswordResetUrl(const std::string& token) const;
};

} // namespace Domains::Identity::Services

