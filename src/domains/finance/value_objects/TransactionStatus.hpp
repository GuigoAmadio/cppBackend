#pragma once
#include <string>
#include <stdexcept>

namespace Finance {

class TransactionStatus {
public:
    enum class Type {
        PENDING,      // Pendente
        COMPLETED,    // Completada
        CANCELLED,    // Cancelada
        RECONCILED    // Reconciliada
    };

    explicit TransactionStatus(Type type) : type_(type) {}
    
    explicit TransactionStatus(const std::string& typeStr) {
        if (typeStr == "pending") type_ = Type::PENDING;
        else if (typeStr == "completed") type_ = Type::COMPLETED;
        else if (typeStr == "cancelled") type_ = Type::CANCELLED;
        else if (typeStr == "reconciled") type_ = Type::RECONCILED;
        else throw std::invalid_argument("Invalid transaction status: " + typeStr);
    }

    std::string toString() const {
        switch (type_) {
            case Type::PENDING: return "pending";
            case Type::COMPLETED: return "completed";
            case Type::CANCELLED: return "cancelled";
            case Type::RECONCILED: return "reconciled";
            default: return "unknown";
        }
    }

    Type getValue() const { return type_; }

    bool isPending() const { return type_ == Type::PENDING; }
    bool isCompleted() const { return type_ == Type::COMPLETED; }
    bool isCancelled() const { return type_ == Type::CANCELLED; }
    bool isReconciled() const { return type_ == Type::RECONCILED; }

private:
    Type type_;
};

} // namespace Finance

