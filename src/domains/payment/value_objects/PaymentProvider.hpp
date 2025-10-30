#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Payment {

/**
 * @brief Provedor de pagamento
 * 
 * Representa os diferentes provedores de pagamento suportados:
 * - STRIPE: Gateway Stripe
 * - PAYPAL: PayPal
 * - PIX: Pix brasileiro (instant payment)
 * - BOLETO: Boleto bancário
 * - CREDIT_CARD: Cartão de crédito direto
 * - MANUAL: Pagamento manual (ex: transferência bancária confirmada manualmente)
 * - SUBSCRIPTION: Pagamento automático (ex: renovação de assinatura)
 */
class PaymentProvider {
public:
    enum class Type {
        STRIPE,
        PAYPAL,
        PIX,
        BOLETO,
        CREDIT_CARD,
        MANUAL,
        SUBSCRIPTION
    };

    explicit PaymentProvider(Type type) : type_(type) {}

    explicit PaymentProvider(const std::string& str) {
        if (str == "stripe") type_ = Type::STRIPE;
        else if (str == "paypal") type_ = Type::PAYPAL;
        else if (str == "pix") type_ = Type::PIX;
        else if (str == "boleto") type_ = Type::BOLETO;
        else if (str == "credit_card") type_ = Type::CREDIT_CARD;
        else if (str == "manual") type_ = Type::MANUAL;
        else if (str == "subscription") type_ = Type::SUBSCRIPTION;
        else throw std::invalid_argument("Invalid payment provider: " + str);
    }

    Type getType() const { return type_; }

    std::string toString() const {
        switch (type_) {
            case Type::STRIPE: return "stripe";
            case Type::PAYPAL: return "paypal";
            case Type::PIX: return "pix";
            case Type::BOLETO: return "boleto";
            case Type::CREDIT_CARD: return "credit_card";
            case Type::MANUAL: return "manual";
            case Type::SUBSCRIPTION: return "subscription";
            default: return "unknown";
        }
    }

    bool operator==(const PaymentProvider& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Domain::Payment

