#ifndef EVENT_STATUS_HPP
#define EVENT_STATUS_HPP

#include <string>
#include <stdexcept>

namespace Domain::Calendar {

class EventStatus {
public:
    enum class Type {
        CONFIRMED,
        TENTATIVE,
        CANCELLED
    };

    explicit EventStatus(Type type) : type_(type) {}
    
    explicit EventStatus(const std::string& statusStr) {
        if (statusStr == "confirmed") type_ = Type::CONFIRMED;
        else if (statusStr == "tentative") type_ = Type::TENTATIVE;
        else if (statusStr == "cancelled") type_ = Type::CANCELLED;
        else throw std::invalid_argument("Invalid event status: " + statusStr);
    }

    std::string toString() const {
        switch (type_) {
            case Type::CONFIRMED: return "confirmed";
            case Type::TENTATIVE: return "tentative";
            case Type::CANCELLED: return "cancelled";
            default: return "confirmed";
        }
    }

    Type getValue() const { return type_; }

    bool isConfirmed() const { return type_ == Type::CONFIRMED; }
    bool isTentative() const { return type_ == Type::TENTATIVE; }
    bool isCancelled() const { return type_ == Type::CANCELLED; }

    bool operator==(const EventStatus& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Domain::Calendar

#endif // EVENT_STATUS_HPP

