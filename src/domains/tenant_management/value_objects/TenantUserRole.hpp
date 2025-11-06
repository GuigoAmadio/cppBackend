#pragma once

#include <string>
#include <stdexcept>

namespace Domains::TenantManagement::ValueObjects {

/**
 * @brief Value Object: TenantUserRole
 * 
 * Representa a role de um usuário em um tenant.
 * Garante que apenas roles válidas podem ser criadas.
 * 
 * Roles válidas:
 * - super_admin: Super administrador (acesso a todos os tenants)
 * - owner: Proprietário do tenant (controle total no tenant)
 * - admin: Administrador do tenant
 * - user: Usuário comum do tenant
 * - viewer: Visualizador (apenas leitura)
 * 
 * @example
 *   TenantUserRole role("admin");           // OK
 *   TenantUserRole invalid("invalid_role"); // Lança exceção
 */
class TenantUserRole {
public:
    enum class Role {
        SUPER_ADMIN,  // Super administrador (acesso global)
        OWNER,        // Proprietário (controle total no tenant)
        ADMIN,        // Administrador do tenant
        USER,         // Usuário comum
        VIEWER        // Visualizador (apenas leitura)
    };

    /**
     * @brief Cria uma TenantUserRole a partir de um enum.
     */
    explicit TenantUserRole(Role role) : role_(role) {}

    /**
     * @brief Cria uma TenantUserRole a partir de uma string.
     * @param roleStr String da role ("super_admin", "owner", "admin", "user", "viewer")
     * @throws std::invalid_argument se role inválida
     */
    explicit TenantUserRole(const std::string& roleStr) {
        if (roleStr == "super_admin") {
            role_ = Role::SUPER_ADMIN;
        } else if (roleStr == "owner") {
            role_ = Role::OWNER;
        } else if (roleStr == "admin") {
            role_ = Role::ADMIN;
        } else if (roleStr == "user") {
            role_ = Role::USER;
        } else if (roleStr == "viewer") {
            role_ = Role::VIEWER;
        } else {
            throw std::invalid_argument("Invalid tenant user role: " + roleStr + 
                ". Valid roles are: super_admin, owner, admin, user, viewer");
        }
    }

    /**
     * @brief Retorna o enum da role.
     */
    Role getRole() const { return role_; }

    /**
     * @brief Retorna a role como string.
     */
    std::string toString() const {
        switch (role_) {
            case Role::SUPER_ADMIN: return "super_admin";
            case Role::OWNER: return "owner";
            case Role::ADMIN: return "admin";
            case Role::USER: return "user";
            case Role::VIEWER: return "viewer";
            default: return "unknown";
        }
    }

    /**
     * @brief Verifica se a role é válida.
     */
    static bool isValid(const std::string& roleStr) {
        return roleStr == "super_admin" || 
               roleStr == "owner" || 
               roleStr == "admin" || 
               roleStr == "user" || 
               roleStr == "viewer";
    }

    /**
     * @brief Retorna a prioridade da role (maior número = maior prioridade).
     * Usado para determinar hierarquia de permissões.
     */
    int getPriority() const {
        switch (role_) {
            case Role::SUPER_ADMIN: return 5;
            case Role::OWNER: return 4;
            case Role::ADMIN: return 3;
            case Role::USER: return 2;
            case Role::VIEWER: return 1;
            default: return 0;
        }
    }

    /**
     * @brief Verifica se esta role pode gerenciar outras roles.
     * Apenas super_admin, owner e admin podem gerenciar roles.
     */
    bool canManageRoles() const {
        return role_ == Role::SUPER_ADMIN || 
               role_ == Role::OWNER || 
               role_ == Role::ADMIN;
    }

    /**
     * @brief Verifica se esta role pode gerenciar membros do tenant.
     * Apenas super_admin, owner e admin podem gerenciar membros.
     */
    bool canManageMembers() const {
        return canManageRoles();
    }

    /**
     * @brief Verifica se esta role pode remover outros usuários.
     * Apenas super_admin e owner podem remover outros usuários.
     */
    bool canRemoveUsers() const {
        return role_ == Role::SUPER_ADMIN || role_ == Role::OWNER;
    }

    /**
     * @brief Verifica se esta role é super_admin.
     */
    bool isSuperAdmin() const {
        return role_ == Role::SUPER_ADMIN;
    }

    /**
     * @brief Verifica se esta role é owner.
     */
    bool isOwner() const {
        return role_ == Role::OWNER;
    }

    /**
     * @brief Verifica se esta role tem prioridade maior ou igual à outra.
     */
    bool hasHigherOrEqualPriorityThan(const TenantUserRole& other) const {
        return getPriority() >= other.getPriority();
    }

    /**
     * @brief Verifica se esta role pode modificar a role de outro usuário.
     * Apenas roles com prioridade maior podem modificar roles com prioridade menor.
     */
    bool canModifyRole(const TenantUserRole& targetRole) const {
        // Não pode modificar sua própria role (exceto owner removendo a si mesmo)
        return getPriority() > targetRole.getPriority();
    }

    // Operadores de comparação
    bool operator==(const TenantUserRole& other) const {
        return role_ == other.role_;
    }

    bool operator!=(const TenantUserRole& other) const {
        return !(*this == other);
    }

    bool operator<(const TenantUserRole& other) const {
        return getPriority() < other.getPriority();
    }

    bool operator>(const TenantUserRole& other) const {
        return getPriority() > other.getPriority();
    }

    bool operator<=(const TenantUserRole& other) const {
        return getPriority() <= other.getPriority();
    }

    bool operator>=(const TenantUserRole& other) const {
        return getPriority() >= other.getPriority();
    }

private:
    Role role_;
};

} // namespace Domains::TenantManagement::ValueObjects

