#pragma once
#include <string>
#include <stdexcept>

namespace Domain {
namespace Order {

class PaymentStatus {
public:
    enum class Value {
        PENDING,
        PAID,
        FAILED,
        REFUNDED
    };
    
    PaymentStatus() : value_(Value::PENDING) {}
    explicit PaymentStatus(Value value) : value_(value) {}
    explicit PaymentStatus(const std::string& str) {
        if (str == "pending") value_ = Value::PENDING;
        else if (str == "paid") value_ = Value::PAID;
        else if (str == "failed") value_ = Value::FAILED;
        else if (str == "refunded") value_ = Value::REFUNDED;
        else throw std::invalid_argument("Invalid payment status: " + str);
    }
    
    std::string toString() const {
        switch (value_) {
            case Value::PENDING: return "pending";
            case Value::PAID: return "paid";
            case Value::FAILED: return "failed";
            case Value::REFUNDED: return "refunded";
            default: return "pending";
        }
    }
    
    Value getValue() const { return value_; }
    
private:
    Value value_;
};

} // namespace Order
} // namespace Domain

