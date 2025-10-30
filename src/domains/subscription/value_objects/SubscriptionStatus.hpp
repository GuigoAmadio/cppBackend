#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Subscription {

class SubscriptionStatus {
public:
    enum class Type {
        TRIAL,      // Período de teste
        ACTIVE,     // Ativa e paga
        PAST_DUE,   // Pagamento atrasado
        CANCELLED,  // Cancelada pelo cliente
        EXPIRED,    // Expirada (não renovada)
        PAUSED      // Pausada temporariamente
    };

    SubscriptionStatus() : type_(Type::TRIAL) {}
    explicit SubscriptionStatus(Type type) : type_(type) {}
    explicit SubscriptionStatus(const std::string& str) {
        if (str == "trial") type_ = Type::TRIAL;
        else if (str == "active") type_ = Type::ACTIVE;
        else if (str == "past_due") type_ = Type::PAST_DUE;
        else if (str == "cancelled") type_ = Type::CANCELLED;
        else if (str == "expired") type_ = Type::EXPIRED;
        else if (str == "paused") type_ = Type::PAUSED;
        else throw std::invalid_argument("Invalid subscription status: " + str);
    }

    std::string toString() const {
        switch (type_) {
            case Type::TRIAL: return "trial";
            case Type::ACTIVE: return "active";
            case Type::PAST_DUE: return "past_due";
            case Type::CANCELLED: return "cancelled";
            case Type::EXPIRED: return "expired";
            case Type::PAUSED: return "paused";
            default: return "trial";
        }
    }

    Type getType() const { return type_; }

    bool isTrial() const { return type_ == Type::TRIAL; }
    bool isActive() const { return type_ == Type::ACTIVE; }
    bool isPastDue() const { return type_ == Type::PAST_DUE; }
    bool isCancelled() const { return type_ == Type::CANCELLED; }
    bool isExpired() const { return type_ == Type::EXPIRED; }
    bool isPaused() const { return type_ == Type::PAUSED; }
    
    bool canBeBilled() const { return isActive() || isTrial(); }
    bool canBeUpgraded() const { return isActive() || isTrial(); }
    bool canBeCancelled() const { return isActive() || isPastDue() || isPaused(); }

private:
    Type type_;
};

} // namespace Domain::Subscription

