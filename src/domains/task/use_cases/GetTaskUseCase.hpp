#pragma once

#include <memory>
#include <optional>
#include "../repositories/TaskRepository.hpp"
#include "../entities/Task.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Task {

struct GetTaskDTO {
    std::string taskId;
    std::string tenantId;
};

class GetTaskUseCase {
public:
    explicit GetTaskUseCase(std::shared_ptr<TaskRepository> repository)
        : repository_(repository) {}

    std::optional<Task> execute(const GetTaskDTO& dto) {
        Core::Utils::Logger::info("[GetTaskUseCase] Fetching task: " + dto.taskId);

        if (dto.taskId.empty() || dto.tenantId.empty()) {
            Core::Utils::Logger::error("[GetTaskUseCase] Task ID and Tenant ID are required");
            return std::nullopt;
        }

        try {
            auto task = repository_->findById(dto.taskId, dto.tenantId);
            
            if (task.has_value()) {
                Core::Utils::Logger::info("[GetTaskUseCase] Task found");
            } else {
                Core::Utils::Logger::warning("[GetTaskUseCase] Task not found");
            }
            
            return task;

        } catch (const std::exception& e) {
            Core::Utils::Logger::error("[GetTaskUseCase] Exception: " + std::string(e.what()));
            return std::nullopt;
        }
    }

private:
    std::shared_ptr<TaskRepository> repository_;
};

} // namespace Domain::Task

