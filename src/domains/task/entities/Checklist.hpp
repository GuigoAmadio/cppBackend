#pragma once

#include <string>
#include <optional>

namespace Domain::Task {

class Checklist {
public:
    Checklist(
        const std::string& id,
        const std::string& taskId,
        const std::string& tenantId,
        const std::string& title
    ) : id_(id),
        taskId_(taskId),
        tenantId_(tenantId),
        title_(title),
        position_(0) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTaskId() const { return taskId_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getTitle() const { return title_; }
    const std::optional<std::string>& getDescription() const { return description_; }
    int getPosition() const { return position_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setTitle(const std::string& title) {
        if (title.empty()) {
            throw std::invalid_argument("Checklist title cannot be empty");
        }
        title_ = title;
    }

    void setDescription(const std::string& description) {
        description_ = description;
    }

    void setPosition(int position) {
        position_ = position;
    }

    void setCreatedAt(const std::string& timestamp) {
        createdAt_ = timestamp;
    }

    void setUpdatedAt(const std::string& timestamp) {
        updatedAt_ = timestamp;
    }

private:
    std::string id_;
    std::string taskId_;
    std::string tenantId_;
    std::string title_;
    std::optional<std::string> description_;
    int position_;
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
};

} // namespace Domain::Task

