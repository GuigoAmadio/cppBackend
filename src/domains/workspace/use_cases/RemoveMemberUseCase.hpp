#pragma once

#include <string>
#include <memory>
#include "../repositories/WorkspaceRepository.hpp"

namespace Domain {
namespace Workspace {

/**
 * @brief Use Case: Remover membro do workspace
 */
class RemoveMemberUseCase {
private:
    std::shared_ptr<WorkspaceRepository> repository_;

public:
    struct Input {
        std::string workspaceId;
        std::string userIdToRemove;
        std::string removedBy;
    };

    struct Output {
        bool success;
        std::string message;
    };

    explicit RemoveMemberUseCase(std::shared_ptr<WorkspaceRepository> repository)
        : repository_(repository) {}

    Output execute(const Input& input) {
        try {
            // Verificar permissão
            auto remover = repository_->findMember(input.workspaceId, input.removedBy);
            if (!remover.has_value() || !remover->canManageMembers()) {
                return Output{false, "Access denied: insufficient permissions"};
            }

            // Não pode remover o último owner
            auto memberToRemove = repository_->findMember(input.workspaceId, input.userIdToRemove);
            if (memberToRemove.has_value() && 
                memberToRemove->getRole().getRole() == MemberRole::Role::OWNER) {
                // Verificar se é o único owner
                int memberCount = repository_->countMembers(input.workspaceId);
                if (memberCount == 1) {
                    return Output{false, "Cannot remove the last owner"};
                }
            }

            // Remover
            bool success = repository_->removeMember(input.workspaceId, input.userIdToRemove);

            if (success) {
                return Output{true, "Member removed successfully"};
            } else {
                return Output{false, "Failed to remove member"};
            }

        } catch (const std::exception& e) {
            return Output{false, std::string("Error: ") + e.what()};
        }
    }
};

} // namespace Workspace
} // namespace Domain

