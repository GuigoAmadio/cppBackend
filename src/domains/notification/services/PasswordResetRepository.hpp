#pragma once

#include "../../../core/database/ConnectionPool.hpp"
#include <string>
#include <memory>
#include <optional>
#include <chrono>

namespace Domains::Notification::Repositories {

struct PasswordResetToken {
    std::string id;
    std::string userId;
    std::string token;
    std::chrono::system_clock::time_point expiresAt;
    std::optional<std::chrono::system_clock::time_point> usedAt;
    std::string ipAddress;
    std::string userAgent;
    std::chrono::system_clock::time_point createdAt;
};

class PasswordResetRepository {
public:
    explicit PasswordResetRepository(std::shared_ptr<Core::Database::ConnectionPool> pool);
    
    /**
     * @brief Cria um novo token de reset
     * @param userId ID do usuário
     * @param token Token único (UUID)
     * @param expiresAt Timestamp de expiração
     * @param ipAddress IP do solicitante (opcional)
     * @param userAgent User-Agent do solicitante (opcional)
     * @return Token criado
     */
    PasswordResetToken create(
        const std::string& userId,
        const std::string& token,
        const std::chrono::system_clock::time_point& expiresAt,
        const std::string& ipAddress = "",
        const std::string& userAgent = ""
    );
    
    /**
     * @brief Busca token pelo valor
     * @param token Token a buscar
     * @return Token encontrado ou nullopt
     */
    std::optional<PasswordResetToken> findByToken(const std::string& token);
    
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

} // namespace Domains::Notification::Repositories

