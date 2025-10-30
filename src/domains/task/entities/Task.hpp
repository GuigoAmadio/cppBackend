#pragma once

#include <string>
#include <vector>
#include <optional>
#include <algorithm>
#include "../value_objects/TaskStatus.hpp"
#include "../value_objects/TaskPriority.hpp"
#include "../value_objects/TaskType.hpp"

namespace Domain::Task {

class Task {
public:
    Task(
        const std::string& id,
        const std::string& tenantId,
        const std::string& title,
        const TaskType& taskType,
        const TaskStatus& status,
        const TaskPriority& priority,
        const std::string& createdBy
    ) : id_(id),
        tenantId_(tenantId),
        title_(title),
        taskType_(taskType),
        status_(status),
        priority_(priority),
        createdBy_(createdBy),
        estimatedHours_(0.0),
        actualHours_(0.0),
        storyPoints_(0),
        position_(0),
        isArchived_(false) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getTitle() const { return title_; }
    const std::optional<std::string>& getDescription() const { return description_; }
    const TaskType& getTaskType() const { return taskType_; }
    const TaskStatus& getStatus() const { return status_; }
    const TaskPriority& getPriority() const { return priority_; }
    const std::string& getCreatedBy() const { return createdBy_; }
    const std::optional<std::string>& getAssignedTo() const { return assignedTo_; }
    const std::optional<std::string>& getReporter() const { return reporter_; }
    const std::optional<std::string>& getWorkspaceId() const { return workspaceId_; }
    const std::optional<std::string>& getProjectId() const { return projectId_; }
    const std::optional<std::string>& getParentTaskId() const { return parentTaskId_; }
    
    double getEstimatedHours() const { return estimatedHours_; }
    double getActualHours() const { return actualHours_; }
    int getStoryPoints() const { return storyPoints_; }
    int getPosition() const { return position_; }
    bool isArchived() const { return isArchived_; }
    
    const std::optional<std::string>& getStartDate() const { return startDate_; }
    const std::optional<std::string>& getDueDate() const { return dueDate_; }
    const std::optional<std::string>& getCompletedAt() const { return completedAt_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getUpdatedAt() const { return updatedAt_; }
    
    const std::vector<std::string>& getDependsOn() const { return dependsOn_; }
    const std::vector<std::string>& getBlocks() const { return blocks_; }
    const std::vector<std::string>& getTags() const { return tags_; }
    const std::vector<std::string>& getLabels() const { return labels_; }

    // Setters
    void setTitle(const std::string& title) {
        if (title.empty()) {
            throw std::invalid_argument("Title cannot be empty");
        }
        if (title.length() > 500) {
            throw std::invalid_argument("Title too long (max 500 characters)");
        }
        title_ = title;
    }

    void setDescription(const std::string& description) {
        description_ = description;
    }

    void setStatus(const TaskStatus& newStatus) {
        if (!status_.canTransitionTo(newStatus)) {
            throw std::invalid_argument("Invalid status transition from " + 
                status_.toString() + " to " + newStatus.toString());
        }
        status_ = newStatus;
        
        // Auto-set completed_at when marked as done
        if (newStatus.getType() == TaskStatus::Type::DONE && !completedAt_.has_value()) {
            // Will be set by use case
        }
    }

    void setPriority(const TaskPriority& priority) {
        priority_ = priority;
    }

    void setAssignedTo(const std::string& userId) {
        assignedTo_ = userId;
    }

    void clearAssignedTo() {
        assignedTo_ = std::nullopt;
    }

    void setReporter(const std::string& userId) {
        reporter_ = userId;
    }

    void setWorkspaceId(const std::string& workspaceId) {
        workspaceId_ = workspaceId;
    }

    void setProjectId(const std::string& projectId) {
        projectId_ = projectId;
    }

    void setParentTaskId(const std::string& parentId) {
        parentTaskId_ = parentId;
    }

    void setEstimatedHours(double hours) {
        if (hours < 0) {
            throw std::invalid_argument("Estimated hours cannot be negative");
        }
        estimatedHours_ = hours;
    }

    void setActualHours(double hours) {
        if (hours < 0) {
            throw std::invalid_argument("Actual hours cannot be negative");
        }
        actualHours_ = hours;
    }

    void addActualHours(double hours) {
        if (hours < 0) {
            throw std::invalid_argument("Hours to add cannot be negative");
        }
        actualHours_ += hours;
    }

    void setStoryPoints(int points) {
        if (points < 0) {
            throw std::invalid_argument("Story points cannot be negative");
        }
        storyPoints_ = points;
    }

    void setPosition(int position) {
        position_ = position;
    }

    void setStartDate(const std::string& date) {
        startDate_ = date;
    }

    void setDueDate(const std::string& date) {
        dueDate_ = date;
    }

    void setCompletedAt(const std::string& timestamp) {
        completedAt_ = timestamp;
    }

    void setCreatedAt(const std::string& timestamp) {
        createdAt_ = timestamp;
    }

    void setUpdatedAt(const std::string& timestamp) {
        updatedAt_ = timestamp;
    }

    void archive() {
        isArchived_ = true;
    }

    void unarchive() {
        isArchived_ = false;
    }

    // Dependencies management
    void addDependency(const std::string& taskId) {
        if (std::find(dependsOn_.begin(), dependsOn_.end(), taskId) == dependsOn_.end()) {
            dependsOn_.push_back(taskId);
        }
    }

    void removeDependency(const std::string& taskId) {
        dependsOn_.erase(
            std::remove(dependsOn_.begin(), dependsOn_.end(), taskId),
            dependsOn_.end()
        );
    }

    void addBlocks(const std::string& taskId) {
        if (std::find(blocks_.begin(), blocks_.end(), taskId) == blocks_.end()) {
            blocks_.push_back(taskId);
        }
    }

    void removeBlocks(const std::string& taskId) {
        blocks_.erase(
            std::remove(blocks_.begin(), blocks_.end(), taskId),
            blocks_.end()
        );
    }

    // Tags & Labels
    void addTag(const std::string& tag) {
        if (std::find(tags_.begin(), tags_.end(), tag) == tags_.end()) {
            tags_.push_back(tag);
        }
    }

    void removeTag(const std::string& tag) {
        tags_.erase(
            std::remove(tags_.begin(), tags_.end(), tag),
            tags_.end()
        );
    }

    void setTags(const std::vector<std::string>& tags) {
        tags_ = tags;
    }

    void addLabel(const std::string& label) {
        if (std::find(labels_.begin(), labels_.end(), label) == labels_.end()) {
            labels_.push_back(label);
        }
    }

    void removeLabel(const std::string& label) {
        labels_.erase(
            std::remove(labels_.begin(), labels_.end(), label),
            labels_.end()
        );
    }

    void setLabels(const std::vector<std::string>& labels) {
        labels_ = labels;
    }

    void setDependsOn(const std::vector<std::string>& deps) {
        dependsOn_ = deps;
    }

    void setBlocks(const std::vector<std::string>& blocks) {
        blocks_ = blocks;
    }

    // Business logic
    bool isOverdue() const {
        if (!dueDate_.has_value() || status_.isCompleted()) {
            return false;
        }
        // Simple check - in production, compare with current timestamp
        return true; // Placeholder
    }

    bool hasAssignee() const {
        return assignedTo_.has_value();
    }

    bool canBeDeleted() const {
        // Cannot delete if has subtasks (would need to check in use case)
        return !status_.isCompleted() || isArchived_;
    }

private:
    std::string id_;
    std::string tenantId_;
    std::string title_;
    std::optional<std::string> description_;
    TaskType taskType_;
    TaskStatus status_;
    TaskPriority priority_;
    std::string createdBy_;
    std::optional<std::string> assignedTo_;
    std::optional<std::string> reporter_;
    std::optional<std::string> workspaceId_;
    std::optional<std::string> projectId_;
    std::optional<std::string> parentTaskId_;
    
    double estimatedHours_;
    double actualHours_;
    int storyPoints_;
    int position_;
    bool isArchived_;
    
    std::optional<std::string> startDate_;
    std::optional<std::string> dueDate_;
    std::optional<std::string> completedAt_;
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
    
    std::vector<std::string> dependsOn_;
    std::vector<std::string> blocks_;
    std::vector<std::string> tags_;
    std::vector<std::string> labels_;
};

} // namespace Domain::Task

