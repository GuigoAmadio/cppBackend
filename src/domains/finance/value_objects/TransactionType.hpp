#pragma once
#include <string>
#include <stdexcept>

namespace Finance {

class TransactionType {
public:
    enum class Type {
        INCOME,    // Receita
        EXPENSE,   // Despesa
        TRANSFER   // Transferência entre contas
    };

    explicit TransactionType(Type type) : type_(type) {}
    
    explicit TransactionType(const std::string& typeStr) {
        if (typeStr == "income") type_ = Type::INCOME;
        else if (typeStr == "expense") type_ = Type::EXPENSE;
        else if (typeStr == "transfer") type_ = Type::TRANSFER;
        else throw std::invalid_argument("Invalid transaction type: " + typeStr);
    }

    std::string toString() const {
        switch (type_) {
            case Type::INCOME: return "income";
            case Type::EXPENSE: return "expense";
            case Type::TRANSFER: return "transfer";
            default: return "unknown";
        }
    }

    Type getValue() const { return type_; }

    bool isIncome() const { return type_ == Type::INCOME; }
    bool isExpense() const { return type_ == Type::EXPENSE; }
    bool isTransfer() const { return type_ == Type::TRANSFER; }

private:
    Type type_;
};

} // namespace Finance

