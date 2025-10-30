#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Task {

class TaskType {
public:
    enum class Type {
        TASK,
        BUG,
        FEATURE,
        STORY,
        EPIC,
        SUBTASK
    };

    explicit TaskType(Type type) : type_(type) {}

    explicit TaskType(const std::string& str) {
        if (str == "task") type_ = Type::TASK;
        else if (str == "bug") type_ = Type::BUG;
        else if (str == "feature") type_ = Type::FEATURE;
        else if (str == "story") type_ = Type::STORY;
        else if (str == "epic") type_ = Type::EPIC;
        else if (str == "subtask") type_ = Type::SUBTASK;
        else throw std::invalid_argument("Invalid task type: " + str);
    }

    Type getType() const { return type_; }

    std::string toString() const {
        switch (type_) {
            case Type::TASK: return "task";
            case Type::BUG: return "bug";
            case Type::FEATURE: return "feature";
            case Type::STORY: return "story";
            case Type::EPIC: return "epic";
            case Type::SUBTASK: return "subtask";
            default: return "unknown";
        }
    }

    bool canHaveSubtasks() const {
        return type_ == Type::TASK || type_ == Type::STORY || type_ == Type::EPIC;
    }

    bool isSubtask() const {
        return type_ == Type::SUBTASK;
    }

    bool operator==(const TaskType& other) const {
        return type_ == other.type_;
    }

    bool operator!=(const TaskType& other) const {
        return !(*this == other);
    }

private:
    Type type_;
};

} // namespace Domain::Task

