#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Subscription {

class BillingCycle {
public:
    enum class Type {
        MONTHLY,    // 1 mês
        QUARTERLY,  // 3 meses
        YEARLY,     // 12 meses
        LIFETIME    // Pagamento único (vitalício)
    };

    BillingCycle() : type_(Type::MONTHLY) {}
    explicit BillingCycle(Type type) : type_(type) {}
    explicit BillingCycle(const std::string& str) {
        if (str == "monthly") type_ = Type::MONTHLY;
        else if (str == "quarterly") type_ = Type::QUARTERLY;
        else if (str == "yearly") type_ = Type::YEARLY;
        else if (str == "lifetime") type_ = Type::LIFETIME;
        else throw std::invalid_argument("Invalid billing cycle: " + str);
    }

    std::string toString() const {
        switch (type_) {
            case Type::MONTHLY: return "monthly";
            case Type::QUARTERLY: return "quarterly";
            case Type::YEARLY: return "yearly";
            case Type::LIFETIME: return "lifetime";
            default: return "monthly";
        }
    }

    Type getType() const { return type_; }

    // Retorna o número de meses do ciclo
    int getMonths() const {
        switch (type_) {
            case Type::MONTHLY: return 1;
            case Type::QUARTERLY: return 3;
            case Type::YEARLY: return 12;
            case Type::LIFETIME: return 0; // Não tem renovação
            default: return 1;
        }
    }

    bool isRecurring() const { return type_ != Type::LIFETIME; }

private:
    Type type_;
};

} // namespace Domain::Subscription

