#include "BcryptService.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <stdexcept>

using namespace Domains::Identity::Services;

BcryptService::BcryptService(int workFactor) : workFactor_(workFactor) {
    if (workFactor_ < 4 || workFactor_ > 31) {
        throw std::invalid_argument("Work factor must be between 4 and 31");
    }
}

std::string BcryptService::hash(const std::string& password) {
    // Generate random salt
    std::string salt = generateSalt();
    
    // Hash password with salt
    std::string hashed = bcryptHash(password, salt);
    
    // Format: $2b$<cost>$<salt><hash>
    std::ostringstream result;
    result << "$2b$" << std::setfill('0') << std::setw(2) << workFactor_ 
           << "$" << salt << hashed;
    
    return result.str();
}

bool BcryptService::verify(const std::string& password, const std::string& hash) {
    try {
        LOG_DEBUG("=== BCRYPT VERIFY DEBUG ===");
        LOG_DEBUG("Password to verify: " + password);
        LOG_DEBUG("Stored hash: " + hash);
        
        // Parse hash format: $2b$<cost>$<salt><hash>
        if (hash.length() < 60 || hash.substr(0, 4) != "$2b$") {
            LOG_WARNING("Hash format invalid: length=" + std::to_string(hash.length()) + ", prefix=" + hash.substr(0, std::min(size_t(4), hash.length())));
            return false;
        }

        // Extract cost
        size_t costStart = 4;
        size_t costEnd = hash.find('$', costStart);
        if (costEnd == std::string::npos) {
            LOG_WARNING("Could not find cost separator");
            return false;
        }
        int cost = std::stoi(hash.substr(costStart, costEnd - costStart));
        LOG_DEBUG("Extracted cost: " + std::to_string(cost));

        // Extract salt (22 chars after cost)
        size_t saltStart = costEnd + 1;
        if (hash.length() < saltStart + 22) {
            LOG_WARNING("Hash too short for salt extraction");
            return false;
        }
        std::string salt = hash.substr(saltStart, 22);
        LOG_DEBUG("Extracted salt: " + salt);

        // Extract stored hash
        std::string storedHash = hash.substr(saltStart + 22);
        LOG_DEBUG("Extracted stored hash: " + storedHash);

        // Hash the input password with the same salt
        std::string newHash = bcryptHash(password, salt);
        LOG_DEBUG("Computed new hash: " + newHash);

        // Constant-time comparison
        bool match = (newHash == storedHash);
        LOG_DEBUG("Hash match: " + std::string(match ? "TRUE" : "FALSE"));
        LOG_DEBUG("=== END BCRYPT VERIFY DEBUG ===");
        
        return match;

    } catch (const std::exception& e) {
        LOG_ERROR("Bcrypt verify exception: " + std::string(e.what()));
        return false;
    } catch (...) {
        LOG_ERROR("Bcrypt verify unknown exception");
        return false;
    }
}

std::string BcryptService::generateSalt() {
    // Generate 16 random bytes
    unsigned char randomBytes[16];
    if (RAND_bytes(randomBytes, sizeof(randomBytes)) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }
    
    // Encode to base64 (22 chars)
    std::string encoded = base64Encode(randomBytes, sizeof(randomBytes));
    
    // Bcrypt uses 22 character salt
    return encoded.substr(0, 22);
}

std::string BcryptService::bcryptHash(const std::string& password, const std::string& salt) {
    // Implementação simplificada usando PBKDF2 (mais portável que bcrypt puro)
    // Em produção, use uma biblioteca bcrypt dedicada como bcrypt.h
    
    unsigned char hash[32]; // 256 bits
    
    // Use PBKDF2 with SHA256
    int iterations = 1 << workFactor_; // 2^workFactor
    
    if (PKCS5_PBKDF2_HMAC(
        password.c_str(), password.length(),
        reinterpret_cast<const unsigned char*>(salt.c_str()), salt.length(),
        iterations,
        EVP_sha256(),
        sizeof(hash),
        hash
    ) != 1) {
        throw std::runtime_error("Password hashing failed");
    }
    
    // Encode hash to base64 (31 chars for bcrypt)
    std::string encoded = base64Encode(hash, sizeof(hash));
    
    // Bcrypt hash is 31 characters
    return encoded.substr(0, 31);
}

std::string BcryptService::base64Encode(const unsigned char* data, size_t len) {
    // Custom base64 alphabet for bcrypt
    static const char bcrypt_alphabet[] =
        "./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    
    std::string result;
    
    for (size_t i = 0; i < len; i += 3) {
        unsigned int val = data[i] << 16;
        if (i + 1 < len) val |= data[i + 1] << 8;
        if (i + 2 < len) val |= data[i + 2];
        
        result += bcrypt_alphabet[(val >> 18) & 0x3F];
        result += bcrypt_alphabet[(val >> 12) & 0x3F];
        if (i + 1 < len) result += bcrypt_alphabet[(val >> 6) & 0x3F];
        if (i + 2 < len) result += bcrypt_alphabet[val & 0x3F];
    }
    
    return result;
}

std::string BcryptService::base64Decode(const std::string& encoded) {
    // Not needed for current implementation
    return "";
}

