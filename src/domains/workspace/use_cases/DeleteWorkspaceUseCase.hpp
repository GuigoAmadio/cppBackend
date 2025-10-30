#pragma once

#include <string>
#include <memory>
#include "../repositories/WorkspaceRepository.hpp"

namespace Domain {
namespace Workspace {

/**
 * @brief Use Case: Deletar workspace
 */
class DeleteWorkspaceUseCase {
private:
    std::shared_ptr<WorkspaceRepository> repository_;

public:
    struct Input {
        std::string workspaceId;
        std::string userId;  // Quem está deletando
    };

    struct Output {
        bool success;
        std::string message;
    };

    explicit DeleteWorkspaceUseCase(std::shared_ptr<WorkspaceRepository> repository)
        : repository_(repository) {}

    Output execute(const Input& input) {
        try {
            // Verificar se usuário é owner
            auto member = repository_->findMember(input.workspaceId, input.userId);
            if (!member.has_value() || !member->canDeleteWorkspace()) {
                return Output{false, "Access denied: only owners can delete workspaces"};
            }

            // Deletar
            bool success = repository_->deleteWorkspace(input.workspaceId);

            if (success) {
                return Output{true, "Workspace deleted successfully"};
            } else {
                return Output{false, "Failed to delete workspace"};
            }

        } catch (const std::exception& e) {
            return Output{false, std::string("Error: ") + e.what()};
        }
    }
};

} // namespace Workspace
} // namespace Domain

