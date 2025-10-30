#include "Password.hpp"
#include <algorithm>
#include <cctype>
#include <vector>

// NOTA: Bcrypt será implementado depois
// Por enquanto, vamos usar um hash simples para não depender de biblioteca externa

namespace Domains::IAM::ValueObjects {

Password::Password(const std::string& plaintext, bool isHashed) {
    if (!isHashed) {
        validate(plaintext);
    }
    password_ = plaintext;
}

std::string Password::hash(int rounds) const {
    // TODO: Implementar bcrypt real
    // Por enquanto, retornar indicador de que precisa implementar
    (void)rounds; // Evitar warning
    return "$2a$12$FAKE_HASH_" + password_; // Placeholder
}

bool Password::verify(const std::string& plaintext, const std::string& hash) {
    // TODO: Implementar bcrypt verification real
    // Por enquanto, comparação simples para testes
    return hash.find(plaintext) != std::string::npos;
}

bool Password::isStrong(const std::string& password) {
    return strengthScore(password) >= 70;
}

int Password::strengthScore(const std::string& password) {
    int score = 0;
    
    // Comprimento (max 40 pontos)
    if (password.length() >= 8) score += 20;
    if (password.length() >= 12) score += 10;
    if (password.length() >= 16) score += 10;
    
    // Complexidade (60 pontos)
    if (hasUppercase(password)) score += 15;
    if (hasLowercase(password)) score += 15;
    if (hasDigit(password)) score += 15;
    if (hasSpecialChar(password)) score += 15;
    
    return std::min(score, 100);
}

void Password::validate(const std::string& password) {
    if (password.empty()) {
        throw std::invalid_argument("Password cannot be empty");
    }
    
    if (password.length() < 8) {
        throw std::invalid_argument("Password must be at least 8 characters long");
    }
    
    if (password.length() > 128) {
        throw std::invalid_argument("Password is too long (max 128 characters)");
    }
    
    // Verificar complexidade mínima
    bool hasUpper = hasUppercase(password);
    bool hasLower = hasLowercase(password);
    bool hasNum = hasDigit(password);
    bool hasSpec = hasSpecialChar(password);
    
    int complexityCount = hasUpper + hasLower + hasNum + hasSpec;
    
    if (complexityCount < 3) {
        throw std::invalid_argument(
            "Password must contain at least 3 of: uppercase, lowercase, digit, special character"
        );
    }
    
    // Verificar senhas comuns (blacklist básica)
    std::vector<std::string> commonPasswords = {
        "password", "12345678", "qwerty", "admin123", "letmein",
        "welcome", "monkey", "1234567890", "password123"
    };
    
    std::string lowerPassword = password;
    std::transform(lowerPassword.begin(), lowerPassword.end(), 
                   lowerPassword.begin(), ::tolower);
    
    for (const auto& common : commonPasswords) {
        if (lowerPassword.find(common) != std::string::npos) {
            throw std::invalid_argument("Password is too common or easily guessable");
        }
    }
}

bool Password::hasUppercase(const std::string& str) {
    return std::any_of(str.begin(), str.end(), [](char c) { return std::isupper(c); });
}

bool Password::hasLowercase(const std::string& str) {
    return std::any_of(str.begin(), str.end(), [](char c) { return std::islower(c); });
}

bool Password::hasDigit(const std::string& str) {
    return std::any_of(str.begin(), str.end(), [](char c) { return std::isdigit(c); });
}

bool Password::hasSpecialChar(const std::string& str) {
    return std::any_of(str.begin(), str.end(), [](char c) {
        return !std::isalnum(c) && !std::isspace(c);
    });
}

} // namespace Domains::IAM::ValueObjects
