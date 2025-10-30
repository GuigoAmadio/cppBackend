#pragma once

#include <string>
#include <memory>
#include "../repositories/WorkspaceRepository.hpp"
#include "../value_objects/MemberRole.hpp"

namespace Domain {
namespace Workspace {

/**
 * @brief Use Case: Atualizar role de membro
 */
class UpdateMemberRoleUseCase {
private:
    std::shared_ptr<WorkspaceRepository> repository_;

public:
    struct Input {
        std::string workspaceId;
        std::string userId;
        std::string newRole;  // "owner", "admin", "member", "guest"
        std::string updatedBy;
    };

    struct Output {
        bool success;
        std::string message;
    };

    explicit UpdateMemberRoleUseCase(std::shared_ptr<WorkspaceRepository> repository)
        : repository_(repository) {}

    Output execute(const Input& input) {
        try {
            // Verificar permissão
            auto updater = repository_->findMember(input.workspaceId, input.updatedBy);
            if (!updater.has_value() || !updater->canManageMembers()) {
                return Output{false, "Access denied: insufficient permissions"};
            }

            // Atualizar role
            MemberRole newRole(input.newRole);
            bool success = repository_->updateMemberRole(
                input.workspaceId,
                input.userId,
                newRole
            );

            if (success) {
                return Output{true, "Member role updated successfully"};
            } else {
                return Output{false, "Failed to update member role"};
            }

        } catch (const std::exception& e) {
            return Output{false, std::string("Error: ") + e.what()};
        }
    }
};

} // namespace Workspace
} // namespace Domain

