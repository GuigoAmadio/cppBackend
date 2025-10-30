#pragma once
#include <string>
#include <stdexcept>

namespace Finance {

class ReconciliationStatus {
public:
    enum class Type {
        PENDING,      // Pendente
        MATCHED,      // Combinado
        UNMATCHED,    // Não combinado
        COMPLETED     // Completado
    };

    explicit ReconciliationStatus(Type type) : type_(type) {}
    
    explicit ReconciliationStatus(const std::string& typeStr) {
        if (typeStr == "pending") type_ = Type::PENDING;
        else if (typeStr == "matched") type_ = Type::MATCHED;
        else if (typeStr == "unmatched") type_ = Type::UNMATCHED;
        else if (typeStr == "completed") type_ = Type::COMPLETED;
        else throw std::invalid_argument("Invalid reconciliation status: " + typeStr);
    }

    std::string toString() const {
        switch (type_) {
            case Type::PENDING: return "pending";
            case Type::MATCHED: return "matched";
            case Type::UNMATCHED: return "unmatched";
            case Type::COMPLETED: return "completed";
            default: return "unknown";
        }
    }

    Type getValue() const { return type_; }

    bool isPending() const { return type_ == Type::PENDING; }
    bool isMatched() const { return type_ == Type::MATCHED; }
    bool isUnmatched() const { return type_ == Type::UNMATCHED; }
    bool isCompleted() const { return type_ == Type::COMPLETED; }

private:
    Type type_;
};

} // namespace Finance

