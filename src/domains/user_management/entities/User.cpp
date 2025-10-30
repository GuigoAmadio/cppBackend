#include "User.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace Domains::UserManagement::Entities {

User::User(
    const std::string& id,
    const Email& email,
    const std::string& passwordHash,
    const std::string& name,
    bool isActive,
    bool emailVerified,
    std::chrono::system_clock::time_point createdAt,
    std::chrono::system_clock::time_point updatedAt,
    std::optional<std::chrono::system_clock::time_point> lastLoginAt
) : id_(id),
    email_(email),
    passwordHash_(passwordHash),
    name_(name),
    isActive_(isActive),
    emailVerified_(emailVerified),
    createdAt_(createdAt),
    updatedAt_(updatedAt),
    lastLoginAt_(lastLoginAt) {
    validateName(name);
}

User User::create(
    const Email& email,
    const Password& password,
    const std::string& name
) {
    auto now = std::chrono::system_clock::now();
    
    return User(
        generateId(),
        email,
        password.value(),  // Usar value() em vez de hash() - o hash já vem pronto do BcryptService
        name,
        true,  // isActive
        false, // emailVerified
        now,   // createdAt
        now,   // updatedAt
        std::nullopt // lastLoginAt
    );
}

bool User::verifyPassword(const std::string& plaintext) const {
    return Password::verify(plaintext, passwordHash_);
}

void User::changePassword(const Password& newPassword) {
    passwordHash_ = newPassword.value();  // Usar value() em vez de hash() - o hash já vem pronto
    updateTimestamp();
}

void User::changeName(const std::string& newName) {
    validateName(newName);
    name_ = newName;
    updateTimestamp();
}

void User::changeEmail(const Email& newEmail) {
    email_ = newEmail;
    emailVerified_ = false; // Requer nova verificação
    updateTimestamp();
}

void User::verifyEmail() {
    emailVerified_ = true;
    updateTimestamp();
}

void User::activate() {
    isActive_ = true;
    updateTimestamp();
}

void User::deactivate() {
    isActive_ = false;
    updateTimestamp();
}

void User::recordLogin() {
    lastLoginAt_ = std::chrono::system_clock::now();
    // Nota: não atualiza updatedAt_ pois login não é modificação do registro
}

void User::updateTimestamp() {
    updatedAt_ = std::chrono::system_clock::now();
}

void User::validateName(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Name cannot be empty");
    }
    
    if (name.length() < 2) {
        throw std::invalid_argument("Name must be at least 2 characters");
    }
    
    if (name.length() > 255) {
        throw std::invalid_argument("Name is too long (max 255 characters)");
    }
    
    // Verificar se contém apenas caracteres válidos (letras, espaços, acentos)
    bool hasLetter = false;
    for (char c : name) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            hasLetter = true;
            break;
        }
    }
    
    if (!hasLetter) {
        throw std::invalid_argument("Name must contain at least one letter");
    }
}

std::string User::generateId() {
    // Gerar UUID v4 simples
    // Em produção, use biblioteca UUID real (boost::uuid ou similar)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex;
    
    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4"; // UUID v4
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }
    
    return ss.str();
}

} // namespace Domains::UserManagement::Entities
