#ifndef EVENT_TYPE_HPP
#define EVENT_TYPE_HPP

#include <string>
#include <stdexcept>

namespace Domain::Calendar {

class EventType {
public:
    enum class Type {
        MEETING,
        TASK,
        REMINDER,
        BIRTHDAY,
        HOLIDAY,
        OTHER
    };

    explicit EventType(Type type) : type_(type) {}
    
    explicit EventType(const std::string& typeStr) {
        if (typeStr == "meeting") type_ = Type::MEETING;
        else if (typeStr == "task") type_ = Type::TASK;
        else if (typeStr == "reminder") type_ = Type::REMINDER;
        else if (typeStr == "birthday") type_ = Type::BIRTHDAY;
        else if (typeStr == "holiday") type_ = Type::HOLIDAY;
        else if (typeStr == "other") type_ = Type::OTHER;
        else throw std::invalid_argument("Invalid event type: " + typeStr);
    }

    std::string toString() const {
        switch (type_) {
            case Type::MEETING: return "meeting";
            case Type::TASK: return "task";
            case Type::REMINDER: return "reminder";
            case Type::BIRTHDAY: return "birthday";
            case Type::HOLIDAY: return "holiday";
            case Type::OTHER: return "other";
            default: return "other";
        }
    }

    Type getValue() const { return type_; }

    bool operator==(const EventType& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Domain::Calendar

#endif // EVENT_TYPE_HPP

