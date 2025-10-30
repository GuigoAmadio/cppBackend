#include "WorkspaceController.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Workspace {

Response WorkspaceController::createWorkspace(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_INFO("WorkspaceController::createWorkspace - START");
        std::string body = req.getBody();
        LOG_INFO("Body: " + body);
        
        // Parse simples do JSON (melhorar depois)
        std::string name, slug, type, description;
        
        // Extrair campos do body
        size_t namePos = body.find("\"name\"");
        if (namePos != std::string::npos) {
            size_t start = body.find("\"", body.find(":", namePos) + 1) + 1;
            size_t end = body.find("\"", start);
            name = body.substr(start, end - start);
        }
        
        size_t slugPos = body.find("\"slug\"");
        if (slugPos != std::string::npos) {
            size_t start = body.find("\"", body.find(":", slugPos) + 1) + 1;
            size_t end = body.find("\"", start);
            slug = body.substr(start, end - start);
        }
        
        size_t typePos = body.find("\"type\"");
        if (typePos != std::string::npos) {
            size_t start = body.find("\"", body.find(":", typePos) + 1) + 1;
            size_t end = body.find("\"", start);
            type = body.substr(start, end - start);
        } else {
            type = "team";  // default
        }

        if (name.empty() || slug.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Name and slug are required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        std::string tenantId = getTenantId(req);
        std::string userId = getUserId(req);
        LOG_INFO("Parsed - name: " + name + ", slug: " + slug + ", type: " + type);
        LOG_INFO("Context - tenantId: [" + tenantId + "], userId: " + userId);

        // Tenant é obrigatório
        if (tenantId.empty()) {
            LOG_WARNING("Workspace creation failed: No tenant_id found");
            json->asObject()["error"] = Core::Json::makeString("Tenant ID is required. User must be associated with a tenant.");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Executar use case
        LOG_INFO("Creating use case...");
        CreateWorkspaceUseCase useCase(repository_);
        LOG_INFO("Executing use case...");
        auto result = useCase.execute({
            tenantId,
            name,
            slug,
            type,
            userId,
            std::nullopt
        });
        LOG_INFO("Use case executed - success: " + std::string(result.success ? "true" : "false"));

        if (result.success) {
            json->asObject()["success"] = Core::Json::makeBool(true);
            json->asObject()["id"] = Core::Json::makeString(result.workspaceId);
            json->asObject()["message"] = Core::Json::makeString(result.message);
            
            LOG_INFO("Workspace created successfully: " + result.workspaceId);
            return Response(StatusCode::Created).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response WorkspaceController::getWorkspace(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string workspaceId = req.getParam("id");
        
        GetWorkspaceUseCase useCase(repository_);
        auto result = useCase.execute({workspaceId, getUserId(req)});

        if (result.success && result.workspace.has_value()) {
            auto ws = result.workspace.value();
            
            auto wsObj = Core::Json::makeObject();
            wsObj->asObject()["id"] = Core::Json::makeString(ws.getId());
            wsObj->asObject()["tenant_id"] = Core::Json::makeString(ws.getTenantId());
            wsObj->asObject()["name"] = Core::Json::makeString(ws.getName());
            wsObj->asObject()["slug"] = Core::Json::makeString(ws.getSlug());
            wsObj->asObject()["type"] = Core::Json::makeString(ws.getType().toString());
            wsObj->asObject()["is_active"] = Core::Json::makeBool(ws.isActive());
            
            if (ws.getDescription().has_value()) {
                wsObj->asObject()["description"] = Core::Json::makeString(*ws.getDescription());
            }
            
            json->asObject()["workspace"] = wsObj;
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::NotFound).json(*json);
        }

    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response WorkspaceController::listWorkspaces(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        ListWorkspacesUseCase useCase(repository_);
        auto result = useCase.execute({
            getTenantId(req),
            getUserId(req),
            100,  // limit
            0     // offset
        });

        if (result.success) {
            auto workspacesArray = Core::Json::makeArray();
            
            for (const auto& ws : result.workspaces) {
                auto wsObj = Core::Json::makeObject();
                wsObj->asObject()["id"] = Core::Json::makeString(ws.getId());
                wsObj->asObject()["name"] = Core::Json::makeString(ws.getName());
                wsObj->asObject()["slug"] = Core::Json::makeString(ws.getSlug());
                wsObj->asObject()["type"] = Core::Json::makeString(ws.getType().toString());
                wsObj->asObject()["is_active"] = Core::Json::makeBool(ws.isActive());
                
                workspacesArray->asArray().push_back(wsObj);
            }
            
            json->asObject()["workspaces"] = workspacesArray;
            json->asObject()["total"] = Core::Json::makeNumber(result.total);
            
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::InternalServerError).json(*json);
        }

    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response WorkspaceController::updateWorkspace(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string workspaceId = req.getParam("id");
        std::string body = req.getBody();
        
        // Parse campos para atualizar
        std::optional<std::string> name, description;
        
        // Extrair name se presente
        size_t namePos = body.find("\"name\"");
        if (namePos != std::string::npos) {
            size_t start = body.find("\"", body.find(":", namePos) + 1) + 1;
            size_t end = body.find("\"", start);
            name = body.substr(start, end - start);
        }

        UpdateWorkspaceUseCase useCase(repository_);
        auto result = useCase.execute({
            workspaceId,
            getUserId(req),
            name,
            description,
            std::nullopt  // settings
        });

        if (result.success) {
            json->asObject()["success"] = Core::Json::makeBool(true);
            json->asObject()["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response WorkspaceController::deleteWorkspace(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string workspaceId = req.getParam("id");

        DeleteWorkspaceUseCase useCase(repository_);
        auto result = useCase.execute({workspaceId, getUserId(req)});

        if (result.success) {
            json->asObject()["success"] = Core::Json::makeBool(true);
            json->asObject()["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::Forbidden).json(*json);
        }

    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response WorkspaceController::inviteMember(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string workspaceId = req.getParam("id");
        std::string body = req.getBody();
        
        std::string userIdToInvite, role = "member";
        
        // Parse user_id e role
        size_t userPos = body.find("\"user_id\"");
        if (userPos != std::string::npos) {
            size_t start = body.find("\"", body.find(":", userPos) + 1) + 1;
            size_t end = body.find("\"", start);
            userIdToInvite = body.substr(start, end - start);
        }
        
        size_t rolePos = body.find("\"role\"");
        if (rolePos != std::string::npos) {
            size_t start = body.find("\"", body.find(":", rolePos) + 1) + 1;
            size_t end = body.find("\"", start);
            role = body.substr(start, end - start);
        }

        if (userIdToInvite.empty()) {
            json->asObject()["error"] = Core::Json::makeString("user_id is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        InviteMemberUseCase useCase(repository_);
        auto result = useCase.execute({
            workspaceId,
            userIdToInvite,
            role,
            getUserId(req)
        });

        if (result.success) {
            json->asObject()["success"] = Core::Json::makeBool(true);
            json->asObject()["member_id"] = Core::Json::makeString(result.memberId);
            json->asObject()["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::Created).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response WorkspaceController::removeMember(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string workspaceId = req.getParam("id");
        std::string userIdToRemove = req.getParam("userId");

        RemoveMemberUseCase useCase(repository_);
        auto result = useCase.execute({
            workspaceId,
            userIdToRemove,
            getUserId(req)
        });

        if (result.success) {
            json->asObject()["success"] = Core::Json::makeBool(true);
            json->asObject()["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response WorkspaceController::updateMemberRole(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string workspaceId = req.getParam("id");
        std::string userId = req.getParam("userId");
        std::string body = req.getBody();
        
        std::string newRole;
        
        // Parse role
        size_t rolePos = body.find("\"role\"");
        if (rolePos != std::string::npos) {
            size_t start = body.find("\"", body.find(":", rolePos) + 1) + 1;
            size_t end = body.find("\"", start);
            newRole = body.substr(start, end - start);
        }

        if (newRole.empty()) {
            json->asObject()["error"] = Core::Json::makeString("role is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        UpdateMemberRoleUseCase useCase(repository_);
        auto result = useCase.execute({
            workspaceId,
            userId,
            newRole,
            getUserId(req)
        });

        if (result.success) {
            json->asObject()["success"] = Core::Json::makeBool(true);
            json->asObject()["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response WorkspaceController::listMembers(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string workspaceId = req.getParam("id");
        std::string userId = getUserId(req);
        
        // Verificar se é membro do workspace
        if (!repository_->isMember(workspaceId, userId)) {
            auto errorJson = Core::Json::makeObject();
            errorJson->asObject()["error"] = Core::Json::makeString("Access denied: not a member");
            return Response(StatusCode::Forbidden).json(*errorJson);
        }
        
        auto members = repository_->findMembers(workspaceId);
        
        auto membersArray = Core::Json::makeArray();
        for (const auto& member : members) {
            auto memberObj = Core::Json::makeObject();
            memberObj->asObject()["id"] = Core::Json::makeString(member.getId());
            memberObj->asObject()["user_id"] = Core::Json::makeString(member.getUserId());
            memberObj->asObject()["role"] = Core::Json::makeString(member.getRole().toString());
            
            if (member.getJoinedAt().has_value()) {
                memberObj->asObject()["joined_at"] = Core::Json::makeString(*member.getJoinedAt());
            }
            
            membersArray->asArray().push_back(memberObj);
        }
        
        json->asObject()["members"] = membersArray;
        json->asObject()["total"] = Core::Json::makeNumber(members.size());
        
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

} // namespace Workspace
} // namespace Domain

