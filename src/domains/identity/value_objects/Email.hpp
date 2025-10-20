#pragma once

#include <string>
#include <stdexcept>
#include <regex>
#include <algorithm>

namespace Domains::Identity::ValueObjects {

/**
 * @brief Value Object para Email.
 * 
 * Garante que o email sempre está em formato válido.
 * Imutável - uma vez criado, não pode ser modificado.
 * 
 * @example
 *   Email email("user@example.com");  // OK
 *   Email invalid("not-an-email");     // Lança exceção
 */
class Email {
public:
    /**
     * @brief Cria um Email validado.
     * @param value String do email
     * @throws std::invalid_argument se email inválido
     */
    explicit Email(const std::string& value);
    
    /**
     * @brief Retorna o email em formato string.
     */
    std::string value() const { return email_; }
    
    /**
     * @brief Retorna o domínio do email (ex: example.com).
     */
    std::string domain() const;
    
    /**
     * @brief Retorna a parte local do email (antes do @).
     */
    std::string localPart() const;
    
    /**
     * @brief Valida se string é um email válido.
     */
    static bool isValid(const std::string& email);
    
    /**
     * @brief Normaliza email (lowercase, trim).
     */
    static std::string normalize(const std::string& email);
    
    // Operadores de comparação
    bool operator==(const Email& other) const { return email_ == other.email_; }
    bool operator!=(const Email& other) const { return !(*this == other); }
    bool operator<(const Email& other) const { return email_ < other.email_; }

private:
    std::string email_;
    
    void validate(const std::string& email);
};

} // namespace Domains::Identity::ValueObjects

