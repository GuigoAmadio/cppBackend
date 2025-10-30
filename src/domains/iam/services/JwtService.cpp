#include "JwtService.hpp"
#include "../../../core/json/JsonParser.hpp"
#include "../../../core/json/JsonValue.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <openssl/hmac.h>
#include <openssl/sha.h>

using namespace Domains::IAM::Services;
using namespace Core::Json;

JwtService::JwtService(const std::string& secret, int expirationMinutes)
    : secret_(secret), expirationMinutes_(expirationMinutes) {}

std::string JwtService::generateToken(
    const std::string& userId,
    const std::string& email,
    const std::string& role
) {
    // Header
    auto header = makeObject();
    header->asObject()["alg"] = makeString("HS256");
    header->asObject()["typ"] = makeString("JWT");
    std::string headerStr = header->toString();

    // Payload
    long long now = getCurrentTimestamp();
    long long exp = now + (expirationMinutes_ * 60);

    auto payload = makeObject();
    payload->asObject()["sub"] = makeString(userId);
    payload->asObject()["email"] = makeString(email);
    payload->asObject()["role"] = makeString(role);
    payload->asObject()["iat"] = makeNumber(static_cast<double>(now));
    payload->asObject()["exp"] = makeNumber(static_cast<double>(exp));
    std::string payloadStr = payload->toString();

    // Encode header and payload
    std::string encodedHeader = base64UrlEncode(headerStr);
    std::string encodedPayload = base64UrlEncode(payloadStr);

    // Create signature
    std::string dataToSign = encodedHeader + "." + encodedPayload;
    std::string signature = hmacSha256(dataToSign, secret_);
    std::string encodedSignature = base64UrlEncode(signature);

    // Combine: header.payload.signature
    return encodedHeader + "." + encodedPayload + "." + encodedSignature;
}

std::string JwtService::generateTokenForTenant(
    const std::string& userId,
    const std::string& email,
    const std::string& tenantId,
    const std::string& tenantSubdomain,
    const std::string& role
) {
    // Create JWT header
    auto header = makeObject();
    header->asObject()["alg"] = makeString("HS256");
    header->asObject()["typ"] = makeString("JWT");
    std::string headerStr = header->toString();

    // Create JWT payload with tenant context
    long long now = getCurrentTimestamp();
    long long exp = now + (expirationMinutes_ * 60);

    auto payload = makeObject();
    payload->asObject()["sub"] = makeString(userId);
    payload->asObject()["email"] = makeString(email);
    payload->asObject()["tenant_id"] = makeString(tenantId);
    payload->asObject()["tenant_subdomain"] = makeString(tenantSubdomain);
    payload->asObject()["role"] = makeString(role);
    payload->asObject()["iat"] = makeNumber(static_cast<double>(now));
    payload->asObject()["exp"] = makeNumber(static_cast<double>(exp));
    std::string payloadStr = payload->toString();

    // Encode header and payload
    std::string encodedHeader = base64UrlEncode(headerStr);
    std::string encodedPayload = base64UrlEncode(payloadStr);

    // Create signature
    std::string dataToSign = encodedHeader + "." + encodedPayload;
    std::string signature = hmacSha256(dataToSign, secret_);
    std::string encodedSignature = base64UrlEncode(signature);

    // Combine: header.payload.signature
    return encodedHeader + "." + encodedPayload + "." + encodedSignature;
}

std::string JwtService::generateRefreshToken(const std::string& userId) {
    // Refresh token com validade de 7 dias
    auto header = makeObject();
    header->asObject()["alg"] = makeString("HS256");
    header->asObject()["typ"] = makeString("JWT");
    std::string headerStr = header->toString();

    long long now = getCurrentTimestamp();
    long long exp = now + (7 * 24 * 60 * 60); // 7 days

    auto payload = makeObject();
    payload->asObject()["sub"] = makeString(userId);
    payload->asObject()["type"] = makeString("refresh");
    payload->asObject()["iat"] = makeNumber(static_cast<double>(now));
    payload->asObject()["exp"] = makeNumber(static_cast<double>(exp));
    std::string payloadStr = payload->toString();

    std::string encodedHeader = base64UrlEncode(headerStr);
    std::string encodedPayload = base64UrlEncode(payloadStr);
    std::string dataToSign = encodedHeader + "." + encodedPayload;
    std::string signature = hmacSha256(dataToSign, secret_);
    std::string encodedSignature = base64UrlEncode(signature);

    return encodedHeader + "." + encodedPayload + "." + encodedSignature;
}

std::map<std::string, std::string> JwtService::validateToken(const std::string& token) {
    std::map<std::string, std::string> result;

    try {
        // Split token
        size_t firstDot = token.find('.');
        size_t secondDot = token.find('.', firstDot + 1);

        if (firstDot == std::string::npos || secondDot == std::string::npos) {
            return result; // Invalid format
        }

        std::string encodedHeader = token.substr(0, firstDot);
        std::string encodedPayload = token.substr(firstDot + 1, secondDot - firstDot - 1);
        std::string encodedSignature = token.substr(secondDot + 1);

        // Verify signature
        std::string dataToVerify = encodedHeader + "." + encodedPayload;
        std::string expectedSignature = hmacSha256(dataToVerify, secret_);
        std::string expectedEncodedSignature = base64UrlEncode(expectedSignature);

        if (encodedSignature != expectedEncodedSignature) {
            return result; // Invalid signature
        }

        // Decode payload
        std::string payloadStr = base64UrlDecode(encodedPayload);
        JsonParser parser(payloadStr);
        auto payload = parser.parse();

        if (!payload || !payload->isObject()) {
            return result;
        }

        auto& obj = payload->asObject();

        // Check expiration
        if (obj.count("exp")) {
            long long exp = static_cast<long long>(obj["exp"]->asNumber());
            long long now = getCurrentTimestamp();
            
            if (now > exp) {
                return result; // Token expired
            }
        }

        // Extract claims
        if (obj.count("sub")) result["sub"] = obj["sub"]->asString();
        if (obj.count("email")) result["email"] = obj["email"]->asString();
        if (obj.count("role")) result["role"] = obj["role"]->asString();
        if (obj.count("type")) result["type"] = obj["type"]->asString();
        
        // Extract tenant context claims
        if (obj.count("tenant_id")) result["tenant_id"] = obj["tenant_id"]->asString();
        if (obj.count("tenant_subdomain")) result["tenant_subdomain"] = obj["tenant_subdomain"]->asString();

        return result;

    } catch (...) {
        return result; // Invalid token
    }
}

bool JwtService::isTokenExpired(const std::string& token) {
    try {
        size_t firstDot = token.find('.');
        size_t secondDot = token.find('.', firstDot + 1);

        if (firstDot == std::string::npos || secondDot == std::string::npos) {
            return true;
        }

        std::string encodedPayload = token.substr(firstDot + 1, secondDot - firstDot - 1);
        std::string payloadStr = base64UrlDecode(encodedPayload);
        
        JsonParser parser(payloadStr);
        auto payload = parser.parse();

        if (!payload || !payload->isObject()) {
            return true;
        }

        auto& obj = payload->asObject();
        if (!obj.count("exp")) {
            return true;
        }

        long long exp = static_cast<long long>(obj["exp"]->asNumber());
        long long now = getCurrentTimestamp();

        return now > exp;

    } catch (...) {
        return true;
    }
}

std::string JwtService::extractUserId(const std::string& token) {
    auto claims = validateToken(token);
    if (claims.count("sub")) {
        return claims["sub"];
    }
    return "";
}

std::string JwtService::base64UrlEncode(const std::string& data) {
    static const char* base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string result;
    int val = 0;
    int valb = -6;
    
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6) {
        result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    
    // Base64 URL: replace + with -, / with _, remove =
    for (char& c : result) {
        if (c == '+') c = '-';
        else if (c == '/') c = '_';
    }
    
    // Remove padding
    size_t pos = result.find('=');
    if (pos != std::string::npos) {
        result = result.substr(0, pos);
    }
    
    return result;
}

std::string JwtService::base64UrlDecode(const std::string& data) {
    static const char base64_table[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,62,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,63,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    };

    std::string input = data;
    // Replace - with +, _ with /
    for (char& c : input) {
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
    }

    // Add padding if needed
    while (input.length() % 4 != 0) {
        input += '=';
    }

    std::string result;
    int val = 0;
    int valb = -8;

    for (unsigned char c : input) {
        if (base64_table[c] == -1) break;
        val = (val << 6) + base64_table[c];
        valb += 6;
        if (valb >= 0) {
            result.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return result;
}

std::string JwtService::hmacSha256(const std::string& data, const std::string& key) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    HMAC(EVP_sha256(), 
         key.c_str(), key.length(),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
         hash, nullptr);
    
    return std::string(reinterpret_cast<char*>(hash), SHA256_DIGEST_LENGTH);
}

long long JwtService::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

