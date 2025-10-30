#pragma once

#include <memory>
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../../../core/json/Json.hpp"
#include "../repositories/WorkspaceRepository.hpp"
#include "../use_cases/CreateWorkspaceUseCase.hpp"
#include "../use_cases/GetWorkspaceUseCase.hpp"
#include "../use_cases/ListWorkspacesUseCase.hpp"
#include "../use_cases/UpdateWorkspaceUseCase.hpp"
#include "../use_cases/DeleteWorkspaceUseCase.hpp"
#include "../use_cases/InviteMemberUseCase.hpp"
#include "../use_cases/RemoveMemberUseCase.hpp"
#include "../use_cases/UpdateMemberRoleUseCase.hpp"

namespace Domain {
namespace Workspace {

using namespace Core::Http;
using namespace Core::Json;

/**
 * @brief Controller: WorkspaceController
 * Endpoints HTTP para gerenciamento de workspaces
 */
class WorkspaceController {
private:
    std::shared_ptr<WorkspaceRepository> repository_;

    // Helper para extrair user context
    std::string getUserId(const Request& req) {
        return req.getCustomData("user_id");
    }

    std::string getTenantId(const Request& req) {
        return req.getCustomData("user_tenant_id");
    }

public:
    explicit WorkspaceController(std::shared_ptr<WorkspaceRepository> repository)
        : repository_(repository) {}

    // POST /api/workspaces - Criar workspace
    Response createWorkspace(const Request& req);

    // GET /api/workspaces/:id - Buscar workspace
    Response getWorkspace(const Request& req);

    // GET /api/workspaces - Listar workspaces
    Response listWorkspaces(const Request& req);

    // PUT /api/workspaces/:id - Atualizar workspace
    Response updateWorkspace(const Request& req);

    // DELETE /api/workspaces/:id - Deletar workspace
    Response deleteWorkspace(const Request& req);

    // POST /api/workspaces/:id/members - Convidar membro
    Response inviteMember(const Request& req);

    // DELETE /api/workspaces/:id/members/:userId - Remover membro
    Response removeMember(const Request& req);

    // PUT /api/workspaces/:id/members/:userId/role - Atualizar role
    Response updateMemberRole(const Request& req);

    // GET /api/workspaces/:id/members - Listar membros
    Response listMembers(const Request& req);
};

} // namespace Workspace
} // namespace Domain

