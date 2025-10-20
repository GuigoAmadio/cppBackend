#pragma once

#include "../../../core/database/ConnectionPool.hpp"
#include <string>
#include <memory>
#include <optional>
#include <chrono>

namespace Domains::Identity::Repositories {

struct EmailVerificationToken {
    std::string id;
    std::string userId;
    std::string token;
    std::chrono::system_clock::time_point expiresAt;
    std::optional<std::chrono::system_clock::time_point> usedAt;
    std::chrono::system_clock::time_point createdAt;
};

class EmailVerificationRepository {
public:
    explicit EmailVerificationRepository(std::shared_ptr<Core::Database::ConnectionPool> pool);
    
    /**
     * @brief Cria um novo token de verificação
     * @param userId ID do usuário
     * @param token Token único (UUID)
     * @param expiresAt Timestamp de expiração
     * @return Token criado
     */
    EmailVerificationToken create(
        const std::string& userId,
        const std::string& token,
        const std::chrono::system_clock::time_point& expiresAt
    );
    
    /**
     * @brief Busca token pelo valor
     * @param token Token a buscar
     * @return Token encontrado ou nullopt
     */
    std::optional<EmailVerificationToken> findByToken(const std::string& token);
    
    /**
     * @brief Marca token como usado
     * @param token Token a marcar
     */
    void markAsUsed(const std::string& token);
    
    /**
     * @brief Deleta tokens expirados (limpeza)
     */
    void deleteExpired();
    
    /**
     * @brief Deleta todos os tokens de um usuário
     * @param userId ID do usuário
     */
    void deleteByUserId(const std::string& userId);

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
};

} // namespace Domains::Identity::Repositories

