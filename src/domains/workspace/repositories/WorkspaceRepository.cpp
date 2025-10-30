#include "WorkspaceRepository.hpp"
#include "../../../core/database/ConnectionPool.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include "../entities/Workspace.hpp"
#include "../entities/WorkspaceMember.hpp"
#include <sstream>

namespace Domain {
namespace Workspace {

class WorkspaceRepositoryImpl : public WorkspaceRepository {
private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;

public:
    explicit WorkspaceRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
        : pool_(pool) {}

    std::string create(
        const std::string& tenantId,
        const std::string& name,
        const std::string& slug,
        const WorkspaceType& type,
        const std::string& createdBy,
        const std::optional<std::string>& description
    ) override {
        auto conn = pool_->acquire();

        std::string query = R"(
            INSERT INTO workspaces (tenant_id, name, slug, type, description, created_by)
            VALUES ($1, $2, $3, $4, $5, $6)
            RETURNING id
        )";

        std::vector<std::string> params = {
            tenantId,
            name,
            slug,
            type.toString(),
            description.value_or(""),
            createdBy
        };

        auto result = conn->executeParams(query, params);

        if (!result.isSuccess() || result.rowCount() == 0) {
            throw std::runtime_error("Failed to create workspace: " + result.getError());
        }

        std::string id = result.getValue(0, 0);
        LOG_INFO("Workspace created: " + id);
        return id;
    }

    std::optional<Workspace> findById(const std::string& id) override {
        auto conn = pool_->acquire();

        std::string query = R"(
            SELECT id, tenant_id, name, slug, type, description, settings, is_active, 
                   created_by, created_at, updated_at
            FROM workspaces
            WHERE id = $1
        )";

        auto result = conn->executeParams(query, {id});

        if (!result.isSuccess() || result.rowCount() == 0) {
            return std::nullopt;
        }

        // Colunas: id, tenant_id, name, slug, type, description, settings, is_active, created_by, created_at, updated_at
        Workspace ws(
            result.getValue(0, 0), // id
            result.getValue(0, 1), // tenant_id
            result.getValue(0, 2), // name
            result.getValue(0, 3), // slug
            WorkspaceType(result.getValue(0, 4)), // type
            result.getValue(0, 8) // created_by
        );
        
        ws.setDescription(result.getValue(0, 5));
        ws.setSettings(result.getValue(0, 6));
        ws.setActive(result.getValue(0, 7) == "t" || result.getValue(0, 7) == "true");
        ws.setCreatedAt(result.getValue(0, 9));
        ws.setUpdatedAt(result.getValue(0, 10));
        
        return ws;
    }

    std::optional<Workspace> findBySlug(const std::string& tenantId, const std::string& slug) override {
        auto conn = pool_->acquire();

        std::string query = R"(
            SELECT id, tenant_id, name, slug, type, description, settings, is_active, created_at, updated_at
            FROM workspaces
            WHERE tenant_id = $1 AND slug = $2
        )";

        auto result = conn->executeParams(query, {tenantId, slug});

        if (!result.isSuccess() || result.rowCount() == 0) {
            return std::nullopt;
        }

        // Colunas: id, tenant_id, name, slug, type, description, settings, is_active, created_at, updated_at
        // NOTA: esta query não retorna created_by, assumindo ""
        Workspace ws(
            result.getValue(0, 0), // id
            result.getValue(0, 1), // tenant_id
            result.getValue(0, 2), // name
            result.getValue(0, 3), // slug
            WorkspaceType(result.getValue(0, 4)), // type
            "" // created_by (não selecionado na query)
        );
        
        ws.setDescription(result.getValue(0, 5));
        ws.setSettings(result.getValue(0, 6));
        ws.setActive(result.getValue(0, 7) == "t" || result.getValue(0, 7) == "true");
        ws.setCreatedAt(result.getValue(0, 8));
        ws.setUpdatedAt(result.getValue(0, 9));
        
        return ws;
    }

    std::vector<Workspace> findByTenant(const std::string& tenantId, int limit, int offset) override {
        auto conn = pool_->acquire();

        std::string query = R"(
            SELECT id, tenant_id, name, slug, type, description, settings, is_active, created_at, updated_at
            FROM workspaces
            WHERE tenant_id = $1
            ORDER BY created_at DESC
            LIMIT $2 OFFSET $3
        )";

        auto result = conn->executeParams(query, {tenantId, std::to_string(limit), std::to_string(offset)});

        std::vector<Workspace> workspaces;
        if (!result.isSuccess()) {
            return workspaces;
        }

        // Colunas: id, tenant_id, name, slug, type, description, settings, is_active, created_at, updated_at
        for (int i = 0; i < result.rowCount(); ++i) {
            Workspace ws(
                result.getValue(i, 0), // id
                result.getValue(i, 1), // tenant_id
                result.getValue(i, 2), // name
                result.getValue(i, 3), // slug
                WorkspaceType(result.getValue(i, 4)), // type
                "" // created_by (não selecionado na query)
            );
            
            ws.setDescription(result.getValue(i, 5));
            ws.setSettings(result.getValue(i, 6));
            ws.setActive(result.getValue(i, 7) == "t" || result.getValue(i, 7) == "true");
            ws.setCreatedAt(result.getValue(i, 8));
            ws.setUpdatedAt(result.getValue(i, 9));
            
            workspaces.push_back(ws);
        }

        return workspaces;
    }

    std::vector<Workspace> findByUser(const std::string& userId, const std::string& tenantId) override {
        auto conn = pool_->acquire();

        std::string query = R"(
            SELECT w.id, w.tenant_id, w.name, w.slug, w.type, w.description, w.settings, w.is_active, w.created_at, w.updated_at
            FROM workspaces w
            INNER JOIN workspace_members wm ON w.id = wm.workspace_id
            WHERE wm.user_id = $1 AND w.tenant_id = $2
            ORDER BY w.created_at DESC
        )";

        auto result = conn->executeParams(query, {userId, tenantId});

        std::vector<Workspace> workspaces;
        if (!result.isSuccess()) {
            return workspaces;
        }

        // Colunas: w.id, w.tenant_id, w.name, w.slug, w.type, w.description, w.settings, w.is_active, w.created_at, w.updated_at
        for (int i = 0; i < result.rowCount(); ++i) {
            Workspace ws(
                result.getValue(i, 0), // id
                result.getValue(i, 1), // tenant_id
                result.getValue(i, 2), // name
                result.getValue(i, 3), // slug
                WorkspaceType(result.getValue(i, 4)), // type
                "" // created_by (não selecionado na query)
            );
            
            ws.setDescription(result.getValue(i, 5));
            ws.setSettings(result.getValue(i, 6));
            ws.setActive(result.getValue(i, 7) == "t" || result.getValue(i, 7) == "true");
            ws.setCreatedAt(result.getValue(i, 8));
            ws.setUpdatedAt(result.getValue(i, 9));
            
            workspaces.push_back(ws);
        }

        return workspaces;
    }

    bool update(
        const std::string& id,
        const std::optional<std::string>& name,
        const std::optional<std::string>& description,
        const std::optional<std::string>& settings
    ) override {
        auto conn = pool_->acquire();

        std::stringstream query;
        query << "UPDATE workspaces SET updated_at = CURRENT_TIMESTAMP";
        
        std::vector<std::string> params;
        int paramCount = 1;

        if (name.has_value()) {
            query << ", name = $" << paramCount++;
            params.push_back(*name);
        }
        if (description.has_value()) {
            query << ", description = $" << paramCount++;
            params.push_back(*description);
        }
        if (settings.has_value()) {
            query << ", settings = $" << paramCount++;
            params.push_back(*settings);
        }

        query << " WHERE id = $" << paramCount;
        params.push_back(id);

        auto result = conn->executeParams(query.str(), params);
        return result.isSuccess();
    }

    bool deleteWorkspace(const std::string& id) override {
        auto conn = pool_->acquire();
        auto result = conn->executeParams("DELETE FROM workspaces WHERE id = $1", {id});
        return result.isSuccess();
    }

    bool setActive(const std::string& id, bool active) override {
        auto conn = pool_->acquire();
        std::string query = "UPDATE workspaces SET is_active = $1, updated_at = CURRENT_TIMESTAMP WHERE id = $2";
        auto result = conn->executeParams(query, {active ? "true" : "false", id});
        return result.isSuccess();
    }

    // ==================== MEMBER OPERATIONS ====================

    std::string addMember(
        const std::string& workspaceId,
        const std::string& userId,
        const MemberRole& role,
        const std::optional<std::string>& invitedBy
    ) override {
        auto conn = pool_->acquire();

        std::string query = R"(
            INSERT INTO workspace_members (workspace_id, user_id, role, invited_by)
            VALUES ($1, $2, $3, $4)
            RETURNING id
        )";

        std::vector<std::string> params = {
            workspaceId,
            userId,
            role.toString(),
            invitedBy.value_or("")
        };

        auto result = conn->executeParams(query, params);

        if (!result.isSuccess() || result.rowCount() == 0) {
            throw std::runtime_error("Failed to add member: " + result.getError());
        }

        return result.getValue(0, 0);
    }

    bool removeMember(const std::string& workspaceId, const std::string& userId) override {
        auto conn = pool_->acquire();
        std::string query = "DELETE FROM workspace_members WHERE workspace_id = $1 AND user_id = $2";
        auto result = conn->executeParams(query, {workspaceId, userId});
        return result.isSuccess();
    }

    bool updateMemberRole(const std::string& workspaceId, const std::string& userId, const MemberRole& newRole) override {
        auto conn = pool_->acquire();
        std::string query = "UPDATE workspace_members SET role = $1 WHERE workspace_id = $2 AND user_id = $3";
        auto result = conn->executeParams(query, {newRole.toString(), workspaceId, userId});
        return result.isSuccess();
    }

    std::optional<WorkspaceMember> findMember(const std::string& workspaceId, const std::string& userId) override {
        auto conn = pool_->acquire();

        std::string query = R"(
            SELECT id, workspace_id, user_id, role, invited_by, joined_at
            FROM workspace_members
            WHERE workspace_id = $1 AND user_id = $2
        )";

        auto result = conn->executeParams(query, {workspaceId, userId});

        if (!result.isSuccess() || result.rowCount() == 0) {
            return std::nullopt;
        }

        // Colunas: id, workspace_id, user_id, role, invited_by, joined_at
        WorkspaceMember member(
            result.getValue(0, 0), // id
            result.getValue(0, 1), // workspace_id
            result.getValue(0, 2), // user_id
            MemberRole(result.getValue(0, 3)) // role
        );
        
        if (!result.isNull(0, 4)) {
            member.setInvitedBy(result.getValue(0, 4));
        }
        if (!result.isNull(0, 5)) {
            member.setJoinedAt(result.getValue(0, 5));
        }
        
        return member;
    }

    std::vector<WorkspaceMember> findMembers(const std::string& workspaceId) override {
        auto conn = pool_->acquire();

        std::string query = R"(
            SELECT id, workspace_id, user_id, role, invited_by, joined_at
            FROM workspace_members
            WHERE workspace_id = $1
            ORDER BY joined_at ASC
        )";

        auto result = conn->executeParams(query, {workspaceId});

        std::vector<WorkspaceMember> members;
        if (!result.isSuccess()) {
            return members;
        }

        for (int i = 0; i < result.rowCount(); ++i) {
            WorkspaceMember member(
                result.getValue(i, 0), // id
                result.getValue(i, 1), // workspace_id
                result.getValue(i, 2), // user_id
                MemberRole(result.getValue(i, 3)) // role
            );
            
            if (!result.isNull(i, 4)) {
                member.setInvitedBy(result.getValue(i, 4));
            }
            if (!result.isNull(i, 5)) {
                member.setJoinedAt(result.getValue(i, 5));
            }
            
            members.push_back(member);
        }

        return members;
    }

    bool isMember(const std::string& workspaceId, const std::string& userId) override {
        auto conn = pool_->acquire();
        std::string query = "SELECT 1 FROM workspace_members WHERE workspace_id = $1 AND user_id = $2";
        auto result = conn->executeParams(query, {workspaceId, userId});
        return result.isSuccess() && result.rowCount() > 0;
    }

    int countMembers(const std::string& workspaceId) override {
        auto conn = pool_->acquire();
        std::string query = "SELECT COUNT(*) FROM workspace_members WHERE workspace_id = $1";
        auto result = conn->executeParams(query, {workspaceId});
        
        if (result.isSuccess() && result.rowCount() > 0) {
            return std::stoi(result.getValue(0, 0));
        }
        return 0;
    }
};

// Factory function
std::shared_ptr<WorkspaceRepository> createWorkspaceRepository(
    std::shared_ptr<Core::Database::ConnectionPool> pool
) {
    return std::make_shared<WorkspaceRepositoryImpl>(pool);
}

} // namespace Workspace
} // namespace Domain

