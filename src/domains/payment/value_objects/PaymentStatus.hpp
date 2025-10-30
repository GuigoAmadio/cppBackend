#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Payment {

/**
 * @brief Status do pagamento
 * 
 * Ciclo de vida:
 * PENDING → PROCESSING → COMPLETED
 *        ↘  FAILED
 *        
 * COMPLETED pode se tornar:
 * - REFUNDED (reembolsado totalmente)
 * - CANCELLED (cancelado)
 */
class PaymentStatus {
public:
    enum class Type {
        PENDING,      // Aguardando processamento
        PROCESSING,   // Sendo processado
        COMPLETED,    // Concluído com sucesso
        FAILED,       // Falhou
        REFUNDED,     // Reembolsado
        CANCELLED     // Cancelado
    };

    explicit PaymentStatus(Type type) : type_(type) {}

    explicit PaymentStatus(const std::string& str) {
        if (str == "pending") type_ = Type::PENDING;
        else if (str == "processing") type_ = Type::PROCESSING;
        else if (str == "completed") type_ = Type::COMPLETED;
        else if (str == "failed") type_ = Type::FAILED;
        else if (str == "refunded") type_ = Type::REFUNDED;
        else if (str == "cancelled") type_ = Type::CANCELLED;
        else throw std::invalid_argument("Invalid payment status: " + str);
    }

    Type getType() const { return type_; }

    std::string toString() const {
        switch (type_) {
            case Type::PENDING: return "pending";
            case Type::PROCESSING: return "processing";
            case Type::COMPLETED: return "completed";
            case Type::FAILED: return "failed";
            case Type::REFUNDED: return "refunded";
            case Type::CANCELLED: return "cancelled";
            default: return "unknown";
        }
    }

    bool operator==(const PaymentStatus& other) const {
        return type_ == other.type_;
    }

    bool isPending() const { return type_ == Type::PENDING; }
    bool isProcessing() const { return type_ == Type::PROCESSING; }
    bool isCompleted() const { return type_ == Type::COMPLETED; }
    bool isFailed() const { return type_ == Type::FAILED; }
    bool isRefunded() const { return type_ == Type::REFUNDED; }
    bool isCancelled() const { return type_ == Type::CANCELLED; }

private:
    Type type_;
};

} // namespace Domain::Payment

