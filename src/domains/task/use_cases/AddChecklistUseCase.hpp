#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../repositories/TaskRepository.hpp"
#include "../entities/Checklist.hpp"
#include "../entities/ChecklistItem.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Task {

struct ChecklistItemData {
    std::string content;
};

struct AddChecklistDTO {
    std::string taskId;
    std::string tenantId;
    std::string title;
    std::vector<ChecklistItemData> items;
};

struct AddChecklistResult {
    bool success;
    std::string checklistId;
    std::string message;
};

class AddChecklistUseCase {
public:
    explicit AddChecklistUseCase(std::shared_ptr<TaskRepository> repository)
        : repository_(repository) {}

    AddChecklistResult execute(const AddChecklistDTO& dto) {
        Core::Utils::Logger::info("[AddChecklistUseCase] Adding checklist to task: " + dto.taskId);

        // Validate
        if (dto.taskId.empty() || dto.tenantId.empty()) {
            return {false, "", "Task ID and Tenant ID are required"};
        }

        if (dto.title.empty()) {
            return {false, "", "Checklist title is required"};
        }

        try {
            // Verify task exists
            auto taskOpt = repository_->findById(dto.taskId, dto.tenantId);
            if (!taskOpt.has_value()) {
                Core::Utils::Logger::error("[AddChecklistUseCase] Task not found");
                return {false, "", "Task not found"};
            }

            // Generate checklist ID
            std::string checklistId = "checklist-" + std::to_string(
                std::chrono::system_clock::now().time_since_epoch().count()
            );

            // Create checklist
            Checklist checklist(checklistId, dto.taskId, dto.tenantId, dto.title);

            // Save checklist
            bool saved = repository_->saveChecklist(checklist);
            if (!saved) {
                Core::Utils::Logger::error("[AddChecklistUseCase] Failed to save checklist");
                return {false, "", "Failed to save checklist"};
            }

            // Create and save items
            int position = 0;
            for (const auto& itemData : dto.items) {
                if (itemData.content.empty()) continue;

                std::string itemId = "item-" + std::to_string(
                    std::chrono::system_clock::now().time_since_epoch().count()
                ) + "-" + std::to_string(position);

                ChecklistItem item(itemId, checklistId, dto.tenantId, itemData.content);
                item.setPosition(position++);

                repository_->saveChecklistItem(item);
            }

            Core::Utils::Logger::info("[AddChecklistUseCase] Checklist added successfully");
            return {true, checklistId, "Checklist added successfully"};

        } catch (const std::exception& e) {
            Core::Utils::Logger::error("[AddChecklistUseCase] Exception: " + std::string(e.what()));
            return {false, "", "Internal error: " + std::string(e.what())};
        }
    }

private:
    std::shared_ptr<TaskRepository> repository_;
};

} // namespace Domain::Task

