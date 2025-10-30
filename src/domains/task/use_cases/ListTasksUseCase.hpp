#pragma once

#include <memory>
#include <vector>
#include <optional>
#include "../repositories/TaskRepository.hpp"
#include "../entities/Task.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Task {

struct ListTasksDTO {
    std::string tenantId;
    std::optional<std::string> workspaceId;
    std::optional<std::string> projectId;
    std::optional<std::string> assignedTo;
    std::optional<std::string> createdBy;
    std::optional<std::string> status;
};

class ListTasksUseCase {
public:
    explicit ListTasksUseCase(std::shared_ptr<TaskRepository> repository)
        : repository_(repository) {}

    std::vector<Task> execute(const ListTasksDTO& dto) {
        Core::Utils::Logger::info("[ListTasksUseCase] Listing tasks");

        if (dto.tenantId.empty()) {
            Core::Utils::Logger::error("[ListTasksUseCase] Tenant ID is required");
            return {};
        }

        try {
            // Apply filters based on provided criteria
            if (dto.workspaceId.has_value() && !dto.workspaceId->empty()) {
                return repository_->findByWorkspace(*dto.workspaceId, dto.tenantId);
            }

            if (dto.projectId.has_value() && !dto.projectId->empty()) {
                return repository_->findByProject(*dto.projectId, dto.tenantId);
            }

            if (dto.assignedTo.has_value() && !dto.assignedTo->empty()) {
                return repository_->findByAssignee(*dto.assignedTo, dto.tenantId);
            }

            if (dto.createdBy.has_value() && !dto.createdBy->empty()) {
                return repository_->findByCreator(*dto.createdBy, dto.tenantId);
            }

            if (dto.status.has_value() && !dto.status->empty()) {
                return repository_->findByStatus(*dto.status, dto.tenantId);
            }

            // No specific filter - return all tasks for tenant
            return repository_->findByTenant(dto.tenantId);

        } catch (const std::exception& e) {
            Core::Utils::Logger::error("[ListTasksUseCase] Exception: " + std::string(e.what()));
            return {};
        }
    }

private:
    std::shared_ptr<TaskRepository> repository_;
};

} // namespace Domain::Task

