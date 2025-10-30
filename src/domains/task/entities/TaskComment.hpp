#pragma once

#include <string>
#include <optional>

namespace Domain::Task {

class TaskComment {
public:
    TaskComment(
        const std::string& id,
        const std::string& taskId,
        const std::string& tenantId,
        const std::string& content,
        const std::string& createdBy
    ) : id_(id),
        taskId_(taskId),
        tenantId_(tenantId),
        content_(content),
        createdBy_(createdBy),
        isEdited_(false),
        isDeleted_(false) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTaskId() const { return taskId_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getContent() const { return content_; }
    const std::string& getCreatedBy() const { return createdBy_; }
    const std::optional<std::string>& getParentCommentId() const { return parentCommentId_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getUpdatedAt() const { return updatedAt_; }
    bool isEdited() const { return isEdited_; }
    bool isDeleted() const { return isDeleted_; }

    // Setters
    void setContent(const std::string& content) {
        if (content.empty()) {
            throw std::invalid_argument("Comment content cannot be empty");
        }
        content_ = content;
        isEdited_ = true;
    }

    void setParentCommentId(const std::string& parentId) {
        parentCommentId_ = parentId;
    }

    void markAsDeleted() {
        isDeleted_ = true;
        content_ = "[deleted]";
    }

    void setCreatedAt(const std::string& timestamp) {
        createdAt_ = timestamp;
    }

    void setUpdatedAt(const std::string& timestamp) {
        updatedAt_ = timestamp;
    }

    bool isReply() const {
        return parentCommentId_.has_value();
    }

private:
    std::string id_;
    std::string taskId_;
    std::string tenantId_;
    std::string content_;
    std::string createdBy_;
    std::optional<std::string> parentCommentId_;
    bool isEdited_;
    bool isDeleted_;
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
};

} // namespace Domain::Task

