#pragma once

#include <string>
#include <stdexcept>

namespace Domain {
namespace Workspace {

/**
 * @brief Value Object: MemberRole
 * Papéis de membros no workspace
 */
class MemberRole {
public:
    enum class Role {
        OWNER,    // Proprietário (controle total)
        ADMIN,    // Administrador
        MEMBER,   // Membro comum
        GUEST     // Convidado (acesso limitado)
    };

    explicit MemberRole(Role role) : role_(role) {}
    explicit MemberRole(const std::string& roleStr) {
        if (roleStr == "owner") role_ = Role::OWNER;
        else if (roleStr == "admin") role_ = Role::ADMIN;
        else if (roleStr == "member") role_ = Role::MEMBER;
        else if (roleStr == "guest") role_ = Role::GUEST;
        else throw std::invalid_argument("Invalid member role: " + roleStr);
    }

    Role getRole() const { return role_; }
    std::string toString() const {
        switch (role_) {
            case Role::OWNER: return "owner";
            case Role::ADMIN: return "admin";
            case Role::MEMBER: return "member";
            case Role::GUEST: return "guest";
            default: return "unknown";
        }
    }

    bool operator==(const MemberRole& other) const {
        return role_ == other.role_;
    }

    // Verificar permissões
    bool canManageMembers() const {
        return role_ == Role::OWNER || role_ == Role::ADMIN;
    }

    bool canManageSettings() const {
        return role_ == Role::OWNER || role_ == Role::ADMIN;
    }

    bool canDeleteWorkspace() const {
        return role_ == Role::OWNER;
    }

private:
    Role role_;
};

} // namespace Workspace
} // namespace Domain

