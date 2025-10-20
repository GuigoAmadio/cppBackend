#pragma once

#include <string>
#include <stdexcept>

namespace Domains::Identity::ValueObjects {

/**
 * @brief Value Object para Password.
 * 
 * Representa uma senha com validações de segurança.
 * Suporta hashing com bcrypt e verificação.
 * 
 * @example
 *   Password pwd("MyStrongP@ss123");
 *   std::string hash = pwd.hash();
 *   bool valid = Password::verify("MyStrongP@ss123", hash);
 */
class Password {
public:
    /**
     * @brief Cria uma Password validada.
     * @param plaintext Senha em texto plano
     * @param isHashed Se true, a senha já está hasheada e não será validada
     * @throws std::invalid_argument se senha não atende critérios (apenas para plain text)
     */
    explicit Password(const std::string& plaintext, bool isHashed = false);
    
    /**
     * @brief Retorna a senha em texto plano.
     * AVISO: Use com cuidado! Nunca salve em texto plano.
     */
    std::string value() const { return password_; }
    
    /**
     * @brief Gera hash bcrypt da senha.
     * @param rounds Número de rounds (default 12)
     * @return Hash bcrypt
     */
    std::string hash(int rounds = 12) const;
    
    /**
     * @brief Verifica se senha corresponde ao hash.
     * @param plaintext Senha em texto plano
     * @param hash Hash bcrypt
     * @return true se corresponde
     */
    static bool verify(const std::string& plaintext, const std::string& hash);
    
    /**
     * @brief Valida força da senha.
     * @return true se senha forte
     */
    static bool isStrong(const std::string& password);
    
    /**
     * @brief Calcula score de força (0-100).
     */
    static int strengthScore(const std::string& password);
    
    /**
     * @brief Valida requisitos mínimos.
     * Lança exceção com mensagem descritiva se inválida.
     */
    static void validate(const std::string& password);

private:
    std::string password_;
    
    static bool hasUppercase(const std::string& str);
    static bool hasLowercase(const std::string& str);
    static bool hasDigit(const std::string& str);
    static bool hasSpecialChar(const std::string& str);
};

} // namespace Domains::Identity::ValueObjects

