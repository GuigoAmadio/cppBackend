#include "PasswordResetRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <iomanip>
#include <sstream>

namespace Domains::Identity::Repositories {

std::string formatTimestampReset(const std::chrono::system_clock::time_point& tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&time_t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::chrono::system_clock::time_point parseTimestampReset(const std::string& str) {
    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

PasswordResetRepository::PasswordResetRepository(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {}

PasswordResetToken PasswordResetRepository::create(
    const std::string& userId,
    const std::string& token,
    const std::chrono::system_clock::time_point& expiresAt,
    const std::string& ipAddress,
    const std::string& userAgent
) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        INSERT INTO password_reset_tokens (user_id, token, expires_at, ip_address, user_agent)
        VALUES ($1, $2, $3, $4, $5)
        RETURNING id, user_id, token, expires_at, used_at, ip_address, user_agent, created_at
    )";
    
    std::string expiresAtStr = formatTimestampReset(expiresAt);
    
    std::vector<std::string> params = {userId, token, expiresAtStr, ipAddress, userAgent};
    auto result = conn->executeParams(sql, params);
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        throw std::runtime_error("Failed to create password reset token");
    }
    
    PasswordResetToken tokenObj;
    tokenObj.id = result.getValue(0, 0);
    tokenObj.userId = result.getValue(0, 1);
    tokenObj.token = result.getValue(0, 2);
    tokenObj.expiresAt = parseTimestampReset(result.getValue(0, 3));
    tokenObj.ipAddress = result.getValue(0, 5);
    tokenObj.userAgent = result.getValue(0, 6);
    tokenObj.createdAt = parseTimestampReset(result.getValue(0, 7));
    
    if (!result.isNull(0, 4)) {
        tokenObj.usedAt = parseTimestampReset(result.getValue(0, 4));
    }
    
    LOG_INFO("Password reset token created for user: " + userId);
    return tokenObj;
}

std::optional<PasswordResetToken> PasswordResetRepository::findByToken(const std::string& token) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT id, user_id, token, expires_at, used_at, ip_address, user_agent, created_at
        FROM password_reset_tokens
        WHERE token = $1
    )";
    
    std::vector<std::string> params = {token};
    auto result = conn->executeParams(sql, params);
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        return std::nullopt;
    }
    
    PasswordResetToken tokenObj;
    tokenObj.id = result.getValue(0, 0);
    tokenObj.userId = result.getValue(0, 1);
    tokenObj.token = result.getValue(0, 2);
    tokenObj.expiresAt = parseTimestampReset(result.getValue(0, 3));
    tokenObj.ipAddress = result.getValue(0, 5);
    tokenObj.userAgent = result.getValue(0, 6);
    tokenObj.createdAt = parseTimestampReset(result.getValue(0, 7));
    
    if (!result.isNull(0, 4)) {
        tokenObj.usedAt = parseTimestampReset(result.getValue(0, 4));
    }
    
    return tokenObj;
}

void PasswordResetRepository::markAsUsed(const std::string& token) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        UPDATE password_reset_tokens
        SET used_at = CURRENT_TIMESTAMP
        WHERE token = $1
    )";
    
    std::vector<std::string> params = {token};
    conn->executeParams(sql, params);
    
    LOG_INFO("Password reset token marked as used: " + token);
}

void PasswordResetRepository::deleteExpired() {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        DELETE FROM password_reset_tokens
        WHERE expires_at < CURRENT_TIMESTAMP
    )";
    
    conn->execute(sql);
    LOG_DEBUG("Deleted expired password reset tokens");
}

void PasswordResetRepository::deleteByUserId(const std::string& userId) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        DELETE FROM password_reset_tokens
        WHERE user_id = $1
    )";
    
    std::vector<std::string> params = {userId};
    conn->executeParams(sql, params);
    
    LOG_DEBUG("Deleted password reset tokens for user: " + userId);
}

} // namespace Domains::Identity::Repositories
