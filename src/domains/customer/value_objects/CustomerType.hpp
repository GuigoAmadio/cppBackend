#pragma once

#include <string>
#include <stdexcept>

namespace Domain {
namespace Customer {

/**
 * @brief Value Object: CustomerType
 * Tipo de cliente (pessoa física, jurídica, etc)
 */
class CustomerType {
public:
    enum class Type {
        INDIVIDUAL,   // Pessoa Física (CPF)
        BUSINESS,     // Pessoa Jurídica (CNPJ)
        FOREIGN       // Estrangeiro
    };

    explicit CustomerType(Type type) : type_(type) {}
    
    explicit CustomerType(const std::string& typeStr) {
        if (typeStr == "individual") {
            type_ = Type::INDIVIDUAL;
        } else if (typeStr == "business") {
            type_ = Type::BUSINESS;
        } else if (typeStr == "foreign") {
            type_ = Type::FOREIGN;
        } else {
            throw std::invalid_argument("Invalid customer type: " + typeStr);
        }
    }

    Type value() const { return type_; }
    
    std::string toString() const {
        switch (type_) {
            case Type::INDIVIDUAL: return "individual";
            case Type::BUSINESS: return "business";
            case Type::FOREIGN: return "foreign";
            default: return "unknown";
        }
    }

    bool isIndividual() const { return type_ == Type::INDIVIDUAL; }
    bool isBusiness() const { return type_ == Type::BUSINESS; }
    bool isForeign() const { return type_ == Type::FOREIGN; }

    bool operator==(const CustomerType& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Customer
} // namespace Domain

