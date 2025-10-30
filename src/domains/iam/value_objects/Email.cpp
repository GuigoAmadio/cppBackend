#include "Email.hpp"
#include <sstream>

namespace Domains::IAM::ValueObjects {

Email::Email(const std::string& value) {
    std::string normalized = normalize(value);
    validate(normalized);
    email_ = normalized;
}

std::string Email::domain() const {
    size_t atPos = email_.find('@');
    if (atPos != std::string::npos && atPos + 1 < email_.length()) {
        return email_.substr(atPos + 1);
    }
    return "";
}

std::string Email::localPart() const {
    size_t atPos = email_.find('@');
    if (atPos != std::string::npos) {
        return email_.substr(0, atPos);
    }
    return "";
}

bool Email::isValid(const std::string& email) {
    if (email.empty() || email.length() > 254) {
        return false;
    }
    
    // Regex básica para validação de email
    // RFC 5322 completo é muito complexo, usamos versão simplificada
    std::regex pattern(
        R"(^[a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$)"
    );
    
    return std::regex_match(email, pattern);
}

std::string Email::normalize(const std::string& email) {
    std::string result = email;
    
    // Trim espaços
    size_t start = result.find_first_not_of(" \t\n\r");
    size_t end = result.find_last_not_of(" \t\n\r");
    
    if (start == std::string::npos) {
        return "";
    }
    
    result = result.substr(start, end - start + 1);
    
    // Converter para lowercase
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    return result;
}

void Email::validate(const std::string& email) {
    if (email.empty()) {
        throw std::invalid_argument("Email cannot be empty");
    }
    
    if (email.length() > 254) {
        throw std::invalid_argument("Email is too long (max 254 characters)");
    }
    
    if (!isValid(email)) {
        throw std::invalid_argument("Invalid email format: " + email);
    }
    
    // Verificar se tem exatamente um @
    size_t atCount = std::count(email.begin(), email.end(), '@');
    if (atCount != 1) {
        throw std::invalid_argument("Email must contain exactly one @ symbol");
    }
    
    // Verificar parte local e domínio
    size_t atPos = email.find('@');
    std::string local = email.substr(0, atPos);
    std::string domain = email.substr(atPos + 1);
    
    if (local.empty()) {
        throw std::invalid_argument("Email local part cannot be empty");
    }
    
    if (local.length() > 64) {
        throw std::invalid_argument("Email local part is too long (max 64 characters)");
    }
    
    if (domain.empty()) {
        throw std::invalid_argument("Email domain cannot be empty");
    }
    
    if (domain.find('.') == std::string::npos) {
        throw std::invalid_argument("Email domain must contain at least one dot");
    }
}

} // namespace Domains::IAM::ValueObjects
