#ifndef REMINDER_TYPE_HPP
#define REMINDER_TYPE_HPP

#include <string>
#include <stdexcept>

namespace Domain::Calendar {

class ReminderType {
public:
    enum class Type {
        NOTIFICATION,
        EMAIL,
        SMS
    };

    explicit ReminderType(Type type) : type_(type) {}
    
    explicit ReminderType(const std::string& typeStr) {
        if (typeStr == "notification") type_ = Type::NOTIFICATION;
        else if (typeStr == "email") type_ = Type::EMAIL;
        else if (typeStr == "sms") type_ = Type::SMS;
        else throw std::invalid_argument("Invalid reminder type: " + typeStr);
    }

    std::string toString() const {
        switch (type_) {
            case Type::NOTIFICATION: return "notification";
            case Type::EMAIL: return "email";
            case Type::SMS: return "sms";
            default: return "notification";
        }
    }

    Type getValue() const { return type_; }

    bool operator==(const ReminderType& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Domain::Calendar

#endif // REMINDER_TYPE_HPP

