#pragma once

#include <string>
#include <stdexcept>

namespace Domain {
namespace Inventory {

/**
 * @brief Value Object: TransactionType
 * Tipos de transação de inventário
 */
class TransactionType {
public:
    enum class Type {
        INBOUND,      // Entrada (compra, produção, devolução de venda)
        OUTBOUND,     // Saída (venda, uso, perda)
        ADJUSTMENT,   // Ajuste manual (contagem, correção)
        TRANSFER      // Transferência entre locais
    };

    explicit TransactionType(Type type) : type_(type) {}
    
    explicit TransactionType(const std::string& typeStr) {
        if (typeStr == "in") {
            type_ = Type::INBOUND;
        } else if (typeStr == "out") {
            type_ = Type::OUTBOUND;
        } else if (typeStr == "adjustment") {
            type_ = Type::ADJUSTMENT;
        } else if (typeStr == "transfer") {
            type_ = Type::TRANSFER;
        } else {
            throw std::invalid_argument("Invalid transaction type: " + typeStr);
        }
    }

    Type value() const { return type_; }
    
    std::string toString() const {
        switch (type_) {
            case Type::INBOUND: return "in";
            case Type::OUTBOUND: return "out";
            case Type::ADJUSTMENT: return "adjustment";
            case Type::TRANSFER: return "transfer";
            default: return "unknown";
        }
    }

    bool operator==(const TransactionType& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Inventory
} // namespace Domain

