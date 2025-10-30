#pragma once

#include <string>
#include <stdexcept>

namespace Domain {
namespace Product {

/**
 * @brief Value Object: ProductType
 * Tipo do produto (physical, digital, service)
 */
class ProductType {
public:
    enum class Type {
        PHYSICAL,  // Produto físico (necessita envio)
        DIGITAL,   // Produto digital (download/acesso online)
        SERVICE    // Serviço (agendamento, consulta, etc)
    };

    explicit ProductType(Type type) : type_(type) {}

    explicit ProductType(const std::string& typeStr) {
        if (typeStr == "physical") {
            type_ = Type::PHYSICAL;
        } else if (typeStr == "digital") {
            type_ = Type::DIGITAL;
        } else if (typeStr == "service") {
            type_ = Type::SERVICE;
        } else {
            throw std::invalid_argument("Invalid product type: " + typeStr);
        }
    }

    Type value() const { return type_; }

    std::string toString() const {
        switch (type_) {
            case Type::PHYSICAL: return "physical";
            case Type::DIGITAL: return "digital";
            case Type::SERVICE: return "service";
            default: return "physical";
        }
    }

    bool isPhysical() const { return type_ == Type::PHYSICAL; }
    bool isDigital() const { return type_ == Type::DIGITAL; }
    bool isService() const { return type_ == Type::SERVICE; }

    bool operator==(const ProductType& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Product
} // namespace Domain

