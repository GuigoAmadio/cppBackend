#pragma once

#include <string>
#include <stdexcept>

namespace Domain {
namespace Inventory {

/**
 * @brief Value Object: ReferenceType
 * Tipo de referência externa (ordem, devolução, etc)
 */
class ReferenceType {
public:
    enum class Type {
        ORDER,        // Pedido/Venda
        PURCHASE,     // Compra
        RETURN,       // Devolução
        PRODUCTION,   // Produção
        ADJUSTMENT,   // Ajuste manual
        TRANSFER,     // Transferência
        LOSS,         // Perda/Quebra
        OTHER         // Outros
    };

    explicit ReferenceType(Type type) : type_(type) {}
    
    explicit ReferenceType(const std::string& typeStr) {
        if (typeStr == "order") {
            type_ = Type::ORDER;
        } else if (typeStr == "purchase") {
            type_ = Type::PURCHASE;
        } else if (typeStr == "return") {
            type_ = Type::RETURN;
        } else if (typeStr == "production") {
            type_ = Type::PRODUCTION;
        } else if (typeStr == "adjustment") {
            type_ = Type::ADJUSTMENT;
        } else if (typeStr == "transfer") {
            type_ = Type::TRANSFER;
        } else if (typeStr == "loss") {
            type_ = Type::LOSS;
        } else if (typeStr == "other" || typeStr.empty()) {
            type_ = Type::OTHER;
        } else {
            throw std::invalid_argument("Invalid reference type: " + typeStr);
        }
    }

    Type value() const { return type_; }
    
    std::string toString() const {
        switch (type_) {
            case Type::ORDER: return "order";
            case Type::PURCHASE: return "purchase";
            case Type::RETURN: return "return";
            case Type::PRODUCTION: return "production";
            case Type::ADJUSTMENT: return "adjustment";
            case Type::TRANSFER: return "transfer";
            case Type::LOSS: return "loss";
            case Type::OTHER: return "other";
            default: return "other";
        }
    }

private:
    Type type_;
};

} // namespace Inventory
} // namespace Domain

