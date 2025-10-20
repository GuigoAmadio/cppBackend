#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <map>

namespace Domains {
namespace Identity {
namespace Services {

/**
 * @brief JWT Service for token generation and validation
 * 
 * Implementação real de JWT usando HMAC-SHA256
 * 
 * Features:
 * - Token generation with custom claims
 * - Token validation and expiration check
 * - Signature verification
 * - Support for refresh tokens
 */
class JwtService {
public:
    /**
     * @brief Construtor
     * @param secret Secret key for signing tokens
     * @param expirationMinutes Token expiration time (default: 60 minutes)
     */
    explicit JwtService(
        const std::string& secret,
        int expirationMinutes = 60
    );

    /**
     * @brief Generate JWT token (deprecated - use generateTokenForTenant)
     * @param userId User ID
     * @param email User email
     * @param role User role (placeholder)
     * @return JWT token string
     */
    std::string generateToken(
        const std::string& userId,
        const std::string& email,
        const std::string& role
    );
    
    /**
     * @brief Generate JWT token with tenant context
     * @param userId User ID
     * @param email User email
     * @param tenantId Tenant ID
     * @param tenantSubdomain Tenant subdomain
     * @param role User role in this tenant
     * @return JWT token string
     */
    std::string generateTokenForTenant(
        const std::string& userId,
        const std::string& email,
        const std::string& tenantId,
        const std::string& tenantSubdomain,
        const std::string& role
    );

    /**
     * @brief Generate refresh token
     * @param userId User ID
     * @return Refresh token string (longer expiration)
     */
    std::string generateRefreshToken(const std::string& userId);

    /**
     * @brief Validate and decode JWT token
     * @param token JWT token to validate
     * @return Map with decoded claims (empty if invalid)
     */
    std::map<std::string, std::string> validateToken(const std::string& token);

    /**
     * @brief Check if token is expired
     * @param token JWT token
     * @return true if expired, false otherwise
     */
    bool isTokenExpired(const std::string& token);

    /**
     * @brief Extract user ID from token
     * @param token JWT token
     * @return User ID (empty if invalid)
     */
    std::string extractUserId(const std::string& token);

private:
    std::string secret_;
    int expirationMinutes_;

    /**
     * @brief Base64 URL encode
     */
    std::string base64UrlEncode(const std::string& data);

    /**
     * @brief Base64 URL decode
     */
    std::string base64UrlDecode(const std::string& data);

    /**
     * @brief Generate HMAC-SHA256 signature
     */
    std::string hmacSha256(const std::string& data, const std::string& key);

    /**
     * @brief Get current timestamp (seconds since epoch)
     */
    long long getCurrentTimestamp();
};

} // namespace Services
} // namespace Identity
} // namespace Domains

