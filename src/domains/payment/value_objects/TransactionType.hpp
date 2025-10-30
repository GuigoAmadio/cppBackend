#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Payment {

/**
 * @brief Tipo de transação de pagamento
 * 
 * - CAPTURE: Captura inicial do pagamento
 * - REFUND: Reembolso (parcial ou total)
 * - CANCEL: Cancelamento
 * - CHARGEBACK: Contestação/estorno forçado
 */
class TransactionType {
public:
    enum class Type {
        CAPTURE,
        REFUND,
        CANCEL,
        CHARGEBACK
    };

    explicit TransactionType(Type type) : type_(type) {}

    explicit TransactionType(const std::string& str) {
        if (str == "capture") type_ = Type::CAPTURE;
        else if (str == "refund") type_ = Type::REFUND;
        else if (str == "cancel") type_ = Type::CANCEL;
        else if (str == "chargeback") type_ = Type::CHARGEBACK;
        else throw std::invalid_argument("Invalid transaction type: " + str);
    }

    Type getType() const { return type_; }

    std::string toString() const {
        switch (type_) {
            case Type::CAPTURE: return "capture";
            case Type::REFUND: return "refund";
            case Type::CANCEL: return "cancel";
            case Type::CHARGEBACK: return "chargeback";
            default: return "unknown";
        }
    }

    bool operator==(const TransactionType& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Domain::Payment

