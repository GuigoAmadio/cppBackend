#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Task {

class TaskStatus {
public:
    enum class Type {
        TODO,
        IN_PROGRESS,
        REVIEW,
        BLOCKED,
        DONE,
        CANCELLED
    };

    explicit TaskStatus(Type type) : type_(type) {}

    explicit TaskStatus(const std::string& str) {
        if (str == "todo") type_ = Type::TODO;
        else if (str == "in_progress") type_ = Type::IN_PROGRESS;
        else if (str == "review") type_ = Type::REVIEW;
        else if (str == "blocked") type_ = Type::BLOCKED;
        else if (str == "done") type_ = Type::DONE;
        else if (str == "cancelled") type_ = Type::CANCELLED;
        else throw std::invalid_argument("Invalid task status: " + str);
    }

    Type getType() const { return type_; }

    std::string toString() const {
        switch (type_) {
            case Type::TODO: return "todo";
            case Type::IN_PROGRESS: return "in_progress";
            case Type::REVIEW: return "review";
            case Type::BLOCKED: return "blocked";
            case Type::DONE: return "done";
            case Type::CANCELLED: return "cancelled";
            default: return "unknown";
        }
    }

    // Validation helpers
    bool canTransitionTo(const TaskStatus& newStatus) const {
        // TODO cannot go directly to DONE
        if (type_ == Type::TODO && newStatus.type_ == Type::DONE) {
            return false;
        }
        // CANCELLED and DONE are terminal states
        if (type_ == Type::CANCELLED || type_ == Type::DONE) {
            return newStatus.type_ == type_; // Can only stay in same state
        }
        return true;
    }

    bool isCompleted() const {
        return type_ == Type::DONE || type_ == Type::CANCELLED;
    }

    bool isActive() const {
        return !isCompleted();
    }

    bool operator==(const TaskStatus& other) const {
        return type_ == other.type_;
    }

    bool operator!=(const TaskStatus& other) const {
        return !(*this == other);
    }

private:
    Type type_;
};

} // namespace Domain::Task

