#pragma once

#include <memory>
#include <string>
#include <optional>
#include "../repositories/TaskRepository.hpp"
#include "../entities/TaskComment.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Task {

struct AddCommentDTO {
    std::string taskId;
    std::string tenantId;
    std::string content;
    std::string createdBy;
    std::optional<std::string> parentCommentId;
};

struct AddCommentResult {
    bool success;
    std::string commentId;
    std::string message;
};

class AddCommentUseCase {
public:
    explicit AddCommentUseCase(std::shared_ptr<TaskRepository> repository)
        : repository_(repository) {}

    AddCommentResult execute(const AddCommentDTO& dto) {
        Core::Utils::Logger::info("[AddCommentUseCase] Adding comment to task: " + dto.taskId);

        // Validate
        if (dto.taskId.empty() || dto.tenantId.empty()) {
            return {false, "", "Task ID and Tenant ID are required"};
        }

        if (dto.content.empty()) {
            return {false, "", "Comment content is required"};
        }

        if (dto.createdBy.empty()) {
            return {false, "", "Creator user ID is required"};
        }

        try {
            // Verify task exists
            auto taskOpt = repository_->findById(dto.taskId, dto.tenantId);
            if (!taskOpt.has_value()) {
                Core::Utils::Logger::error("[AddCommentUseCase] Task not found");
                return {false, "", "Task not found"};
            }

            // Generate comment ID
            std::string commentId = "comment-" + std::to_string(
                std::chrono::system_clock::now().time_since_epoch().count()
            );

            // Create comment
            TaskComment comment(commentId, dto.taskId, dto.tenantId, dto.content, dto.createdBy);

            if (dto.parentCommentId.has_value() && !dto.parentCommentId->empty()) {
                comment.setParentCommentId(*dto.parentCommentId);
            }

            // Save
            bool saved = repository_->saveComment(comment);

            if (saved) {
                Core::Utils::Logger::info("[AddCommentUseCase] Comment added successfully");
                return {true, commentId, "Comment added successfully"};
            } else {
                Core::Utils::Logger::error("[AddCommentUseCase] Failed to save comment");
                return {false, "", "Failed to save comment"};
            }

        } catch (const std::exception& e) {
            Core::Utils::Logger::error("[AddCommentUseCase] Exception: " + std::string(e.what()));
            return {false, "", "Internal error: " + std::string(e.what())};
        }
    }

private:
    std::shared_ptr<TaskRepository> repository_;
};

} // namespace Domain::Task

