#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "../value_objects/WorkspaceType.hpp"
#include "../value_objects/MemberRole.hpp"
#include "../entities/Workspace.hpp"
#include "../entities/WorkspaceMember.hpp"

// Forward declaration
namespace Core { namespace Database { class ConnectionPool; } }

namespace Domain {
namespace Workspace {

/**
 * @brief Repository Interface: WorkspaceRepository
 * Acesso a dados de workspaces e membros
 */
class WorkspaceRepository {
public:
    virtual ~WorkspaceRepository() = default;

    // ==================== WORKSPACE OPERATIONS ====================
    
    /**
     * @brief Criar novo workspace
     */
    virtual std::string create(
        const std::string& tenantId,
        const std::string& name,
        const std::string& slug,
        const WorkspaceType& type,
        const std::string& createdBy,
        const std::optional<std::string>& description = std::nullopt
    ) = 0;

    /**
     * @brief Buscar workspace por ID
     */
    virtual std::optional<Workspace> findById(const std::string& id) = 0;

    /**
     * @brief Buscar workspace por slug dentro de um tenant
     */
    virtual std::optional<Workspace> findBySlug(
        const std::string& tenantId, 
        const std::string& slug
    ) = 0;

    /**
     * @brief Listar todos os workspaces de um tenant
     */
    virtual std::vector<Workspace> findByTenant(
        const std::string& tenantId,
        int limit = 100,
        int offset = 0
    ) = 0;

    /**
     * @brief Listar workspaces de um usuário
     */
    virtual std::vector<Workspace> findByUser(
        const std::string& userId,
        const std::string& tenantId
    ) = 0;

    /**
     * @brief Atualizar workspace
     */
    virtual bool update(
        const std::string& id,
        const std::optional<std::string>& name,
        const std::optional<std::string>& description,
        const std::optional<std::string>& settings
    ) = 0;

    /**
     * @brief Deletar workspace
     */
    virtual bool deleteWorkspace(const std::string& id) = 0;

    /**
     * @brief Ativar/desativar workspace
     */
    virtual bool setActive(const std::string& id, bool active) = 0;

    // ==================== MEMBER OPERATIONS ====================

    /**
     * @brief Adicionar membro ao workspace
     */
    virtual std::string addMember(
        const std::string& workspaceId,
        const std::string& userId,
        const MemberRole& role,
        const std::optional<std::string>& invitedBy = std::nullopt
    ) = 0;

    /**
     * @brief Remover membro do workspace
     */
    virtual bool removeMember(const std::string& workspaceId, const std::string& userId) = 0;

    /**
     * @brief Atualizar role de um membro
     */
    virtual bool updateMemberRole(
        const std::string& workspaceId,
        const std::string& userId,
        const MemberRole& newRole
    ) = 0;

    /**
     * @brief Buscar membro específico
     */
    virtual std::optional<WorkspaceMember> findMember(
        const std::string& workspaceId,
        const std::string& userId
    ) = 0;

    /**
     * @brief Listar todos os membros de um workspace
     */
    virtual std::vector<WorkspaceMember> findMembers(const std::string& workspaceId) = 0;

    /**
     * @brief Verificar se usuário é membro do workspace
     */
    virtual bool isMember(const std::string& workspaceId, const std::string& userId) = 0;

    /**
     * @brief Contar membros de um workspace
     */
    virtual int countMembers(const std::string& workspaceId) = 0;
};

// Factory function
std::shared_ptr<WorkspaceRepository> createWorkspaceRepository(
    std::shared_ptr<Core::Database::ConnectionPool> pool
);

} // namespace Workspace
} // namespace Domain

