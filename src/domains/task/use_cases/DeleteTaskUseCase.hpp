#pragma once

#include <memory>
#include "../repositories/TaskRepository.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Task {

struct DeleteTaskDTO {
    std::string taskId;
    std::string tenantId;
};

struct DeleteTaskResult {
    bool success;
    std::string message;
};

class DeleteTaskUseCase {
public:
    explicit DeleteTaskUseCase(std::shared_ptr<TaskRepository> repository)
        : repository_(repository) {}

    DeleteTaskResult execute(const DeleteTaskDTO& dto) {
        Core::Utils::Logger::info("[DeleteTaskUseCase] Deleting task: " + dto.taskId);

        if (dto.taskId.empty() || dto.tenantId.empty()) {
            return {false, "Task ID and Tenant ID are required"};
        }

        try {
            // Check if task exists
            auto taskOpt = repository_->findById(dto.taskId, dto.tenantId);
            if (!taskOpt.has_value()) {
                Core::Utils::Logger::error("[DeleteTaskUseCase] Task not found");
                return {false, "Task not found"};
            }

            // Check if it has subtasks (should prevent deletion)
            auto subtasks = repository_->findByParent(dto.taskId, dto.tenantId);
            if (!subtasks.empty()) {
                Core::Utils::Logger::error("[DeleteTaskUseCase] Cannot delete task with subtasks");
                return {false, "Cannot delete task with subtasks. Delete subtasks first."};
            }

            // Delete the task
            bool deleted = repository_->remove(dto.taskId, dto.tenantId);

            if (deleted) {
                Core::Utils::Logger::info("[DeleteTaskUseCase] Task deleted successfully");
                return {true, "Task deleted successfully"};
            } else {
                Core::Utils::Logger::error("[DeleteTaskUseCase] Failed to delete task");
                return {false, "Failed to delete task"};
            }

        } catch (const std::exception& e) {
            Core::Utils::Logger::error("[DeleteTaskUseCase] Exception: " + std::string(e.what()));
            return {false, "Internal error: " + std::string(e.what())};
        }
    }

private:
    std::shared_ptr<TaskRepository> repository_;
};

} // namespace Domain::Task

