#pragma once

#include <memory>
#include <string>
#include <optional>
#include <vector>
#include "../repositories/TaskRepository.hpp"
#include "../entities/Task.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Task {

struct CreateTaskDTO {
    std::string tenantId;
    std::string title;
    std::string taskType;  // "task", "bug", "feature", etc.
    std::string priority;  // "low", "medium", "high", "urgent"
    std::string createdBy;
    std::optional<std::string> description;
    std::optional<std::string> workspaceId;
    std::optional<std::string> projectId;
    std::optional<std::string> assignedTo;
    std::optional<std::string> parentTaskId;
    std::optional<double> estimatedHours;
    std::optional<int> storyPoints;
    std::optional<std::string> dueDate;
    std::optional<std::vector<std::string>> tags;
    std::optional<std::vector<std::string>> labels;
};

struct CreateTaskResult {
    bool success;
    std::string taskId;
    std::string message;
};

class CreateTaskUseCase {
public:
    explicit CreateTaskUseCase(std::shared_ptr<TaskRepository> repository)
        : repository_(repository) {}

    CreateTaskResult execute(const CreateTaskDTO& dto) {
        Core::Utils::Logger::info("[CreateTaskUseCase] Starting - title: " + dto.title);

        // Validate required fields
        if (dto.tenantId.empty()) {
            Core::Utils::Logger::error("[CreateTaskUseCase] Tenant ID is required");
            return {false, "", "Tenant ID is required"};
        }

        if (dto.title.empty()) {
            Core::Utils::Logger::error("[CreateTaskUseCase] Title is required");
            return {false, "", "Title is required"};
        }

        if (dto.title.length() > 500) {
            Core::Utils::Logger::error("[CreateTaskUseCase] Title too long");
            return {false, "", "Title too long (max 500 characters)"};
        }

        if (dto.createdBy.empty()) {
            Core::Utils::Logger::error("[CreateTaskUseCase] Creator user ID is required");
            return {false, "", "Creator user ID is required"};
        }

        try {
            // Generate unique ID
            std::string taskId = "task-" + std::to_string(
                std::chrono::system_clock::now().time_since_epoch().count()
            );

            Core::Utils::Logger::info("[CreateTaskUseCase] Generated task ID: " + taskId);

            // Create value objects with validation
            TaskType taskType(dto.taskType.empty() ? "task" : dto.taskType);
            TaskStatus status(TaskStatus::Type::TODO); // New tasks always start as TODO
            TaskPriority priority(dto.priority.empty() ? "medium" : dto.priority);

            // Create task entity
            Task task(taskId, dto.tenantId, dto.title, taskType, status, priority, dto.createdBy);

            // Set optional fields
            if (dto.description.has_value() && !dto.description->empty()) {
                task.setDescription(*dto.description);
            }

            if (dto.workspaceId.has_value() && !dto.workspaceId->empty()) {
                task.setWorkspaceId(*dto.workspaceId);
            }

            if (dto.projectId.has_value() && !dto.projectId->empty()) {
                task.setProjectId(*dto.projectId);
            }

            if (dto.assignedTo.has_value() && !dto.assignedTo->empty()) {
                task.setAssignedTo(*dto.assignedTo);
            }

            if (dto.parentTaskId.has_value() && !dto.parentTaskId->empty()) {
                task.setParentTaskId(*dto.parentTaskId);
            }

            if (dto.estimatedHours.has_value()) {
                task.setEstimatedHours(*dto.estimatedHours);
            }

            if (dto.storyPoints.has_value()) {
                task.setStoryPoints(*dto.storyPoints);
            }

            if (dto.dueDate.has_value() && !dto.dueDate->empty()) {
                task.setDueDate(*dto.dueDate);
            }

            if (dto.tags.has_value()) {
                task.setTags(*dto.tags);
            }

            if (dto.labels.has_value()) {
                task.setLabels(*dto.labels);
            }

            // Save to repository
            Core::Utils::Logger::info("[CreateTaskUseCase] Saving task to repository");
            bool saved = repository_->save(task);

            if (saved) {
                Core::Utils::Logger::info("[CreateTaskUseCase] Task created successfully: " + taskId);
                return {true, taskId, "Task created successfully"};
            } else {
                Core::Utils::Logger::error("[CreateTaskUseCase] Failed to save task");
                return {false, "", "Failed to save task"};
            }

        } catch (const std::invalid_argument& e) {
            Core::Utils::Logger::error("[CreateTaskUseCase] Validation error: " + std::string(e.what()));
            return {false, "", std::string(e.what())};
        } catch (const std::exception& e) {
            Core::Utils::Logger::error("[CreateTaskUseCase] Exception: " + std::string(e.what()));
            return {false, "", "Internal error: " + std::string(e.what())};
        }
    }

private:
    std::shared_ptr<TaskRepository> repository_;
};

} // namespace Domain::Task

