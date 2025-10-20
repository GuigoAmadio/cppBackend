#pragma once

#include <string>

namespace Domains {
namespace Identity {
namespace Services {

/**
 * @brief Bcrypt Service for password hashing
 * 
 * Implementação de bcrypt usando OpenSSL
 * 
 * Features:
 * - Password hashing with salt
 * - Password verification
 * - Configurable work factor (cost)
 * - Secure random salt generation
 */
class BcryptService {
public:
    /**
     * @brief Construtor
     * @param workFactor Work factor/cost (default: 12, range: 4-31)
     *                   Maior = mais seguro mas mais lento
     */
    explicit BcryptService(int workFactor = 12);

    /**
     * @brief Hash a password
     * @param password Plain text password
     * @return Hashed password (includes salt and cost)
     */
    std::string hash(const std::string& password);

    /**
     * @brief Verify password against hash
     * @param password Plain text password
     * @param hash Hashed password to verify against
     * @return true if password matches, false otherwise
     */
    bool verify(const std::string& password, const std::string& hash);

    /**
     * @brief Generate random salt
     * @return Random salt string
     */
    std::string generateSalt();

private:
    int workFactor_;

    /**
     * @brief Perform bcrypt hashing
     */
    std::string bcryptHash(const std::string& password, const std::string& salt);

    /**
     * @brief Base64 encode for bcrypt
     */
    std::string base64Encode(const unsigned char* data, size_t len);

    /**
     * @brief Base64 decode for bcrypt
     */
    std::string base64Decode(const std::string& encoded);
};

} // namespace Services
} // namespace Identity
} // namespace Domains

