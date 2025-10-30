#pragma once

#include <memory>
#include <optional>
#include "../repositories/TaskRepository.hpp"
#include "../entities/Task.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Task {

struct UpdateTaskDTO {
    std::string taskId;
    std::string tenantId;
    std::optional<std::string> title;
    std::optional<std::string> description;
    std::optional<std::string> priority;
    std::optional<std::string> assignedTo;
    std::optional<double> estimatedHours;
    std::optional<int> storyPoints;
    std::optional<std::string> dueDate;
    std::optional<std::string> workspaceId;
    std::optional<std::string> projectId;
};

struct UpdateTaskResult {
    bool success;
    std::string message;
};

class UpdateTaskUseCase {
public:
    explicit UpdateTaskUseCase(std::shared_ptr<TaskRepository> repository)
        : repository_(repository) {}

    UpdateTaskResult execute(const UpdateTaskDTO& dto) {
        Core::Utils::Logger::info("[UpdateTaskUseCase] Updating task: " + dto.taskId);

        if (dto.taskId.empty() || dto.tenantId.empty()) {
            return {false, "Task ID and Tenant ID are required"};
        }

        try {
            // Fetch existing task
            auto taskOpt = repository_->findById(dto.taskId, dto.tenantId);
            if (!taskOpt.has_value()) {
                Core::Utils::Logger::error("[UpdateTaskUseCase] Task not found");
                return {false, "Task not found"};
            }

            Task task = *taskOpt;

            // Update fields
            if (dto.title.has_value()) {
                task.setTitle(*dto.title);
            }

            if (dto.description.has_value()) {
                task.setDescription(*dto.description);
            }

            if (dto.priority.has_value()) {
                task.setPriority(TaskPriority(*dto.priority));
            }

            if (dto.assignedTo.has_value()) {
                if (dto.assignedTo->empty()) {
                    task.clearAssignedTo();
                } else {
                    task.setAssignedTo(*dto.assignedTo);
                }
            }

            if (dto.estimatedHours.has_value()) {
                task.setEstimatedHours(*dto.estimatedHours);
            }

            if (dto.storyPoints.has_value()) {
                task.setStoryPoints(*dto.storyPoints);
            }

            if (dto.dueDate.has_value()) {
                task.setDueDate(*dto.dueDate);
            }

            if (dto.workspaceId.has_value()) {
                task.setWorkspaceId(*dto.workspaceId);
            }

            if (dto.projectId.has_value()) {
                task.setProjectId(*dto.projectId);
            }

            // Save updated task
            bool updated = repository_->update(task);

            if (updated) {
                Core::Utils::Logger::info("[UpdateTaskUseCase] Task updated successfully");
                return {true, "Task updated successfully"};
            } else {
                Core::Utils::Logger::error("[UpdateTaskUseCase] Failed to update task");
                return {false, "Failed to update task"};
            }

        } catch (const std::invalid_argument& e) {
            Core::Utils::Logger::error("[UpdateTaskUseCase] Validation error: " + std::string(e.what()));
            return {false, std::string(e.what())};
        } catch (const std::exception& e) {
            Core::Utils::Logger::error("[UpdateTaskUseCase] Exception: " + std::string(e.what()));
            return {false, "Internal error: " + std::string(e.what())};
        }
    }

private:
    std::shared_ptr<TaskRepository> repository_;
};

} // namespace Domain::Task

