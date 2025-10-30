#pragma once

#include <string>
#include <optional>

namespace Domain::Task {

class ChecklistItem {
public:
    ChecklistItem(
        const std::string& id,
        const std::string& checklistId,
        const std::string& tenantId,
        const std::string& content
    ) : id_(id),
        checklistId_(checklistId),
        tenantId_(tenantId),
        content_(content),
        isCompleted_(false),
        position_(0) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getChecklistId() const { return checklistId_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getContent() const { return content_; }
    bool isCompleted() const { return isCompleted_; }
    const std::optional<std::string>& getCompletedBy() const { return completedBy_; }
    const std::optional<std::string>& getCompletedAt() const { return completedAt_; }
    int getPosition() const { return position_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setContent(const std::string& content) {
        if (content.empty()) {
            throw std::invalid_argument("Checklist item content cannot be empty");
        }
        content_ = content;
    }

    void complete(const std::string& userId, const std::string& timestamp) {
        isCompleted_ = true;
        completedBy_ = userId;
        completedAt_ = timestamp;
    }

    void uncomplete() {
        isCompleted_ = false;
        completedBy_ = std::nullopt;
        completedAt_ = std::nullopt;
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
    std::string checklistId_;
    std::string tenantId_;
    std::string content_;
    bool isCompleted_;
    std::optional<std::string> completedBy_;
    std::optional<std::string> completedAt_;
    int position_;
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
};

} // namespace Domain::Task

