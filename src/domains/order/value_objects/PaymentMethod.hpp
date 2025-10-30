#pragma once
#include <string>
#include <stdexcept>

namespace Domain {
namespace Order {

class PaymentMethod {
public:
    enum class Value {
        CREDIT_CARD,
        DEBIT_CARD,
        PIX,
        BOLETO,
        CASH
    };
    
    PaymentMethod() : value_(Value::PIX) {}
    explicit PaymentMethod(Value value) : value_(value) {}
    explicit PaymentMethod(const std::string& str) {
        if (str == "credit_card") value_ = Value::CREDIT_CARD;
        else if (str == "debit_card") value_ = Value::DEBIT_CARD;
        else if (str == "pix") value_ = Value::PIX;
        else if (str == "boleto") value_ = Value::BOLETO;
        else if (str == "cash") value_ = Value::CASH;
        else throw std::invalid_argument("Invalid payment method: " + str);
    }
    
    std::string toString() const {
        switch (value_) {
            case Value::CREDIT_CARD: return "credit_card";
            case Value::DEBIT_CARD: return "debit_card";
            case Value::PIX: return "pix";
            case Value::BOLETO: return "boleto";
            case Value::CASH: return "cash";
            default: return "pix";
        }
    }
    
    Value getValue() const { return value_; }
    
private:
    Value value_;
};

} // namespace Order
} // namespace Domain

