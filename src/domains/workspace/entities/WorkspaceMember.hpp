#pragma once

#include <string>
#include <optional>
#include "../value_objects/MemberRole.hpp"

namespace Domain {
namespace Workspace {

/**
 * @brief Entity: WorkspaceMember
 * Representa um membro de um workspace
 */
class WorkspaceMember {
public:
    WorkspaceMember(
        const std::string& id,
        const std::string& workspaceId,
        const std::string& userId,
        const MemberRole& role
    ) : id_(id),
        workspaceId_(workspaceId),
        userId_(userId),
        role_(role) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getWorkspaceId() const { return workspaceId_; }
    const std::string& getUserId() const { return userId_; }
    const MemberRole& getRole() const { return role_; }
    const std::optional<std::string>& getInvitedBy() const { return invitedBy_; }
    const std::optional<std::string>& getJoinedAt() const { return joinedAt_; }

    // Setters
    void setRole(const MemberRole& role) { role_ = role; }
    void setInvitedBy(const std::string& invitedBy) { invitedBy_ = invitedBy; }
    void setJoinedAt(const std::string& joinedAt) { joinedAt_ = joinedAt; }

    // Business logic
    bool canManageMembers() const {
        return role_.canManageMembers();
    }

    bool canManageSettings() const {
        return role_.canManageSettings();
    }

    bool canDeleteWorkspace() const {
        return role_.canDeleteWorkspace();
    }

    bool canPromoteOthers() const {
        return role_.getRole() == MemberRole::Role::OWNER;
    }

private:
    std::string id_;
    std::string workspaceId_;
    std::string userId_;
    MemberRole role_;
    std::optional<std::string> invitedBy_;
    std::optional<std::string> joinedAt_;
};

} // namespace Workspace
} // namespace Domain

