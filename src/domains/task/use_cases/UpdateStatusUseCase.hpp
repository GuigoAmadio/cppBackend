#pragma once

#include <memory>
#include "../repositories/TaskRepository.hpp"
#include "../entities/Task.hpp"
#include "../../../core/utils/Logger.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Domain::Task {

struct UpdateStatusDTO {
    std::string taskId;
    std::string tenantId;
    std::string newStatus;
};

struct UpdateStatusResult {
    bool success;
    std::string message;
};

class UpdateStatusUseCase {
public:
    explicit UpdateStatusUseCase(std::shared_ptr<TaskRepository> repository)
        : repository_(repository) {}

    UpdateStatusResult execute(const UpdateStatusDTO& dto) {
        Core::Utils::Logger::info("[UpdateStatusUseCase] Updating status for task: " + dto.taskId + " to " + dto.newStatus);

        if (dto.taskId.empty() || dto.tenantId.empty()) {
            return {false, "Task ID and Tenant ID are required"};
        }

        if (dto.newStatus.empty()) {
            return {false, "New status is required"};
        }

        try {
            // Fetch existing task
            auto taskOpt = repository_->findById(dto.taskId, dto.tenantId);
            if (!taskOpt.has_value()) {
                Core::Utils::Logger::error("[UpdateStatusUseCase] Task not found");
                return {false, "Task not found"};
            }

            Task task = *taskOpt;

            // Validate and set new status
            TaskStatus newStatus(dto.newStatus);
            task.setStatus(newStatus);

            // If status is DONE, set completed_at timestamp
            if (newStatus.getType() == TaskStatus::Type::DONE && !task.getCompletedAt().has_value()) {
                auto now = std::chrono::system_clock::now();
                auto time_t_now = std::chrono::system_clock::to_time_t(now);
                std::ostringstream oss;
                oss << std::put_time(std::gmtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
                task.setCompletedAt(oss.str());
            }

            // Save updated task
            bool updated = repository_->update(task);

            if (updated) {
                Core::Utils::Logger::info("[UpdateStatusUseCase] Task status updated successfully");
                return {true, "Task status updated successfully"};
            } else {
                Core::Utils::Logger::error("[UpdateStatusUseCase] Failed to update task status");
                return {false, "Failed to update task status"};
            }

        } catch (const std::invalid_argument& e) {
            Core::Utils::Logger::error("[UpdateStatusUseCase] Validation error: " + std::string(e.what()));
            return {false, std::string(e.what())};
        } catch (const std::exception& e) {
            Core::Utils::Logger::error("[UpdateStatusUseCase] Exception: " + std::string(e.what()));
            return {false, "Internal error: " + std::string(e.what())};
        }
    }

private:
    std::shared_ptr<TaskRepository> repository_;
};

} // namespace Domain::Task

