#pragma once

#include "../value_objects/Email.hpp"
#include "../value_objects/Password.hpp"
#include <string>
#include <chrono>
#include <optional>
#include <memory>

namespace Domains::Identity::Entities {

using namespace ValueObjects;

/**
 * @brief User Entity (DDD).
 * 
 * Representa um usuário do sistema com toda sua lógica de negócio.
 * Entity = tem identidade única (ID) e ciclo de vida.
 * 
 * @example
 *   User user = User::create(
 *       Email("user@example.com"),
 *       Password("SecureP@ss123"),
 *       "John Doe"
 *   );
 */
class User {
public:
    /**
     * @brief Cria usuário existente (do banco de dados).
     * @note Role agora é específico por tenant na tabela user_tenants
     */
    User(
        const std::string& id,
        const Email& email,
        const std::string& passwordHash,
        const std::string& name,
        bool isActive,
        bool emailVerified,
        std::chrono::system_clock::time_point createdAt,
        std::chrono::system_clock::time_point updatedAt,
        std::optional<std::chrono::system_clock::time_point> lastLoginAt = std::nullopt
    );
    
    /**
     * @brief Factory method para criar novo usuário.
     * Gera ID automaticamente e define timestamps.
     * @note Role é atribuído depois na tabela user_tenants
     */
    static User create(
        const Email& email,
        const Password& password,
        const std::string& name
    );
    
    // Getters
    std::string getId() const { return id_; }
    Email getEmail() const { return email_; }
    std::string getPasswordHash() const { return passwordHash_; }
    std::string getName() const { return name_; }
    bool isActive() const { return isActive_; }
    bool isEmailVerified() const { return emailVerified_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt_; }
    std::chrono::system_clock::time_point getUpdatedAt() const { return updatedAt_; }
    std::optional<std::chrono::system_clock::time_point> getLastLoginAt() const { return lastLoginAt_; }
    
    // Business logic methods
    
    /**
     * @brief Verifica se senha corresponde ao hash armazenado.
     */
    bool verifyPassword(const std::string& plaintext) const;
    
    /**
     * @brief Atualiza senha (gera novo hash).
     */
    void changePassword(const Password& newPassword);
    
    /**
     * @brief Atualiza nome do usuário.
     */
    void changeName(const std::string& newName);
    
    /**
     * @brief Atualiza email (requer nova verificação).
     */
    void changeEmail(const Email& newEmail);
    
    /**
     * @brief Marca email como verificado.
     */
    void verifyEmail();
    
    /**
     * @brief Ativa usuário.
     */
    void activate();
    
    /**
     * @brief Desativa usuário (soft delete).
     */
    void deactivate();
    
    /**
     * @brief Registra último login.
     */
    void recordLogin();

private:
    std::string id_;
    Email email_;
    std::string passwordHash_;
    std::string name_;
    bool isActive_;
    bool emailVerified_;
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point updatedAt_;
    std::optional<std::chrono::system_clock::time_point> lastLoginAt_;
    
    void updateTimestamp();
    void validateName(const std::string& name);
    static std::string generateId();
};

} // namespace Domains::Identity::Entities

