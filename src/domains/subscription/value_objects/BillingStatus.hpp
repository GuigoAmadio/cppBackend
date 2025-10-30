#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Subscription {

class BillingStatus {
public:
    enum class Type {
        PENDING,   // Cobrança pendente
        PAID,      // Pago com sucesso
        FAILED,    // Falha no pagamento
        REFUNDED   // Reembolsado
    };

    BillingStatus() : type_(Type::PENDING) {}
    explicit BillingStatus(Type type) : type_(type) {}
    explicit BillingStatus(const std::string& str) {
        if (str == "pending") type_ = Type::PENDING;
        else if (str == "paid") type_ = Type::PAID;
        else if (str == "failed") type_ = Type::FAILED;
        else if (str == "refunded") type_ = Type::REFUNDED;
        else throw std::invalid_argument("Invalid billing status: " + str);
    }

    std::string toString() const {
        switch (type_) {
            case Type::PENDING: return "pending";
            case Type::PAID: return "paid";
            case Type::FAILED: return "failed";
            case Type::REFUNDED: return "refunded";
            default: return "pending";
        }
    }

    Type getType() const { return type_; }

    bool isPending() const { return type_ == Type::PENDING; }
    bool isPaid() const { return type_ == Type::PAID; }
    bool isFailed() const { return type_ == Type::FAILED; }
    bool isRefunded() const { return type_ == Type::REFUNDED; }

private:
    Type type_;
};

} // namespace Domain::Subscription

