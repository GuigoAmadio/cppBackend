#pragma once
#include <string>
#include <stdexcept>

namespace Finance {

class BudgetPeriod {
public:
    enum class Type {
        DAILY,
        WEEKLY,
        MONTHLY,
        QUARTERLY,
        YEARLY,
        CUSTOM
    };

    explicit BudgetPeriod(Type type) : type_(type) {}
    
    explicit BudgetPeriod(const std::string& typeStr) {
        if (typeStr == "daily") type_ = Type::DAILY;
        else if (typeStr == "weekly") type_ = Type::WEEKLY;
        else if (typeStr == "monthly") type_ = Type::MONTHLY;
        else if (typeStr == "quarterly") type_ = Type::QUARTERLY;
        else if (typeStr == "yearly") type_ = Type::YEARLY;
        else if (typeStr == "custom") type_ = Type::CUSTOM;
        else throw std::invalid_argument("Invalid budget period: " + typeStr);
    }

    std::string toString() const {
        switch (type_) {
            case Type::DAILY: return "daily";
            case Type::WEEKLY: return "weekly";
            case Type::MONTHLY: return "monthly";
            case Type::QUARTERLY: return "quarterly";
            case Type::YEARLY: return "yearly";
            case Type::CUSTOM: return "custom";
            default: return "unknown";
        }
    }

    Type getValue() const { return type_; }

    bool isDaily() const { return type_ == Type::DAILY; }
    bool isWeekly() const { return type_ == Type::WEEKLY; }
    bool isMonthly() const { return type_ == Type::MONTHLY; }
    bool isQuarterly() const { return type_ == Type::QUARTERLY; }
    bool isYearly() const { return type_ == Type::YEARLY; }
    bool isCustom() const { return type_ == Type::CUSTOM; }

private:
    Type type_;
};

} // namespace Finance

