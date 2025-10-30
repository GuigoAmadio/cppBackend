#ifndef RECURRENCE_FREQUENCY_HPP
#define RECURRENCE_FREQUENCY_HPP

#include <string>
#include <stdexcept>

namespace Domain::Calendar {

class RecurrenceFrequency {
public:
    enum class Type {
        DAILY,
        WEEKLY,
        MONTHLY,
        YEARLY
    };

    explicit RecurrenceFrequency(Type type) : type_(type) {}
    
    explicit RecurrenceFrequency(const std::string& freqStr) {
        if (freqStr == "daily") type_ = Type::DAILY;
        else if (freqStr == "weekly") type_ = Type::WEEKLY;
        else if (freqStr == "monthly") type_ = Type::MONTHLY;
        else if (freqStr == "yearly") type_ = Type::YEARLY;
        else throw std::invalid_argument("Invalid recurrence frequency: " + freqStr);
    }

    std::string toString() const {
        switch (type_) {
            case Type::DAILY: return "daily";
            case Type::WEEKLY: return "weekly";
            case Type::MONTHLY: return "monthly";
            case Type::YEARLY: return "yearly";
            default: return "daily";
        }
    }

    Type getValue() const { return type_; }

    bool operator==(const RecurrenceFrequency& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Domain::Calendar

#endif // RECURRENCE_FREQUENCY_HPP

