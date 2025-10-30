#pragma once
#include <string>
#include <stdexcept>

namespace Finance {

class AccountType {
public:
    enum class Type {
        CHECKING,      // Conta corrente
        SAVINGS,       // Poupança
        CASH,          // Dinheiro
        CREDIT_CARD,   // Cartão de crédito
        INVESTMENT     // Investimento
    };

    explicit AccountType(Type type) : type_(type) {}
    
    explicit AccountType(const std::string& typeStr) {
        if (typeStr == "checking") type_ = Type::CHECKING;
        else if (typeStr == "savings") type_ = Type::SAVINGS;
        else if (typeStr == "cash") type_ = Type::CASH;
        else if (typeStr == "credit_card") type_ = Type::CREDIT_CARD;
        else if (typeStr == "investment") type_ = Type::INVESTMENT;
        else throw std::invalid_argument("Invalid account type: " + typeStr);
    }

    std::string toString() const {
        switch (type_) {
            case Type::CHECKING: return "checking";
            case Type::SAVINGS: return "savings";
            case Type::CASH: return "cash";
            case Type::CREDIT_CARD: return "credit_card";
            case Type::INVESTMENT: return "investment";
            default: return "unknown";
        }
    }

    Type getValue() const { return type_; }

    bool isChecking() const { return type_ == Type::CHECKING; }
    bool isSavings() const { return type_ == Type::SAVINGS; }
    bool isCash() const { return type_ == Type::CASH; }
    bool isCreditCard() const { return type_ == Type::CREDIT_CARD; }
    bool isInvestment() const { return type_ == Type::INVESTMENT; }

private:
    Type type_;
};

} // namespace Finance

