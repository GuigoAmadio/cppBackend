#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Task {

class TaskPriority {
public:
    enum class Type {
        LOW,
        MEDIUM,
        HIGH,
        URGENT
    };

    explicit TaskPriority(Type type) : type_(type) {}

    explicit TaskPriority(const std::string& str) {
        if (str == "low") type_ = Type::LOW;
        else if (str == "medium") type_ = Type::MEDIUM;
        else if (str == "high") type_ = Type::HIGH;
        else if (str == "urgent") type_ = Type::URGENT;
        else throw std::invalid_argument("Invalid task priority: " + str);
    }

    Type getType() const { return type_; }

    std::string toString() const {
        switch (type_) {
            case Type::LOW: return "low";
            case Type::MEDIUM: return "medium";
            case Type::HIGH: return "high";
            case Type::URGENT: return "urgent";
            default: return "unknown";
        }
    }

    int getNumericValue() const {
        switch (type_) {
            case Type::LOW: return 1;
            case Type::MEDIUM: return 2;
            case Type::HIGH: return 3;
            case Type::URGENT: return 4;
            default: return 0;
        }
    }

    bool operator<(const TaskPriority& other) const {
        return getNumericValue() < other.getNumericValue();
    }

    bool operator>(const TaskPriority& other) const {
        return getNumericValue() > other.getNumericValue();
    }

    bool operator==(const TaskPriority& other) const {
        return type_ == other.type_;
    }

    bool operator!=(const TaskPriority& other) const {
        return !(*this == other);
    }

private:
    Type type_;
};

} // namespace Domain::Task

