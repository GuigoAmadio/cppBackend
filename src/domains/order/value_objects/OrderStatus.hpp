#pragma once
#include <string>
#include <stdexcept>

namespace Domain {
namespace Order {

class OrderStatus {
public:
    enum class Value {
        PENDING,
        CONFIRMED,
        PROCESSING,
        SHIPPED,
        DELIVERED,
        CANCELLED
    };
    
    OrderStatus() : value_(Value::PENDING) {}
    explicit OrderStatus(Value value) : value_(value) {}
    explicit OrderStatus(const std::string& str) {
        if (str == "pending") value_ = Value::PENDING;
        else if (str == "confirmed") value_ = Value::CONFIRMED;
        else if (str == "processing") value_ = Value::PROCESSING;
        else if (str == "shipped") value_ = Value::SHIPPED;
        else if (str == "delivered") value_ = Value::DELIVERED;
        else if (str == "cancelled") value_ = Value::CANCELLED;
        else throw std::invalid_argument("Invalid order status: " + str);
    }
    
    std::string toString() const {
        switch (value_) {
            case Value::PENDING: return "pending";
            case Value::CONFIRMED: return "confirmed";
            case Value::PROCESSING: return "processing";
            case Value::SHIPPED: return "shipped";
            case Value::DELIVERED: return "delivered";
            case Value::CANCELLED: return "cancelled";
            default: return "pending";
        }
    }
    
    Value getValue() const { return value_; }
    bool operator==(const OrderStatus& other) const { return value_ == other.value_; }
    bool operator!=(const OrderStatus& other) const { return value_ != other.value_; }
    
private:
    Value value_;
};

} // namespace Order
} // namespace Domain

