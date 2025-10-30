#pragma once

#include <string>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace Domain {
namespace Product {

/**
 * @brief Value Object: ProductCode
 * Código único que identifica um produto (ex: "PROD-001", "ABC123")
 */
class ProductCode {
public:
    explicit ProductCode(const std::string& code) {
        if (!isValid(code)) {
            throw std::invalid_argument("Invalid product code: must be 3-100 chars, alphanumeric + dashes/underscores");
        }
        code_ = normalize(code);
    }

    const std::string& value() const { return code_; }
    std::string toString() const { return code_; }

    bool operator==(const ProductCode& other) const {
        return code_ == other.code_;
    }

private:
    std::string code_;

    static bool isValid(const std::string& code) {
        if (code.length() < 3 || code.length() > 100) {
            return false;
        }

        // Permitir apenas: letras, números, hífens, underscores
        return std::all_of(code.begin(), code.end(), [](char c) {
            return std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_';
        });
    }

    static std::string normalize(const std::string& code) {
        // Converter para uppercase para consistência
        std::string normalized = code;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                     [](unsigned char c) { return std::toupper(c); });
        return normalized;
    }
};

} // namespace Product
} // namespace Domain

