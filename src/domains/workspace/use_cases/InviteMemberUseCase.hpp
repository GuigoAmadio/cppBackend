#pragma once

#include <string>
#include <memory>
#include "../repositories/WorkspaceRepository.hpp"
#include "../value_objects/MemberRole.hpp"

namespace Domain {
namespace Workspace {

/**
 * @brief Use Case: Convidar membro para workspace
 */
class InviteMemberUseCase {
private:
    std::shared_ptr<WorkspaceRepository> repository_;

public:
    struct Input {
        std::string workspaceId;
        std::string userIdToInvite;
        std::string role;  // "owner", "admin", "member", "guest"
        std::string invitedBy;
    };

    struct Output {
        bool success;
        std::string memberId;
        std::string message;
    };

    explicit InviteMemberUseCase(std::shared_ptr<WorkspaceRepository> repository)
        : repository_(repository) {}

    Output execute(const Input& input) {
        try {
            // Verificar se quem está convidando tem permissão
            auto inviter = repository_->findMember(input.workspaceId, input.invitedBy);
            if (!inviter.has_value() || !inviter->canManageMembers()) {
                return Output{false, "", "Access denied: insufficient permissions"};
            }

            // Verificar se usuário já é membro
            if (repository_->isMember(input.workspaceId, input.userIdToInvite)) {
                return Output{false, "", "User is already a member"};
            }

            // Adicionar membro
            MemberRole role(input.role);
            std::string memberId = repository_->addMember(
                input.workspaceId,
                input.userIdToInvite,
                role,
                input.invitedBy
            );

            return Output{true, memberId, "Member invited successfully"};

        } catch (const std::exception& e) {
            return Output{false, "", std::string("Error: ") + e.what()};
        }
    }
};

} // namespace Workspace
} // namespace Domain

