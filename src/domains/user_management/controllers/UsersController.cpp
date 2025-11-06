#include "UsersController.hpp"
#include "../../../core/json/Json.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::UserManagement::Controllers {

UsersController::UsersController(
    std::shared_ptr<GetUserUseCase> getUserUseCase,
    std::shared_ptr<UpdateUserUseCase> updateUserUseCase,
    std::shared_ptr<ChangePasswordUseCase> changePasswordUseCase,
    std::shared_ptr<DeleteUserUseCase> deleteUserUseCase,
    std::shared_ptr<ListUsersUseCase> listUsersUseCase
) : getUserUseCase_(getUserUseCase),
    updateUserUseCase_(updateUserUseCase),
    changePasswordUseCase_(changePasswordUseCase),
    deleteUserUseCase_(deleteUserUseCase),
    listUsersUseCase_(listUsersUseCase) {}

Response UsersController::me(const Request& req) {
    try {
        std::string userId = req.getCustomData("user_id");
        std::string userEmail = req.getCustomData("user_email");
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string role = req.getCustomData("user_role");

        LOG_DEBUG("🔵 [USERS] GET /api/me - User ID: " + userId);

        auto userOpt = getUserUseCase_->execute(userId);

        if (!userOpt.has_value()) {
            LOG_ERROR("❌ [USERS] User not found: " + userId);
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("User not found");
            return Response(StatusCode::NotFound).json(*error);
        }

        auto user = userOpt.value();
        auto json = userToJson(user);
        
        // Adicionar informações do token/tenant
        if (!tenantId.empty()) {
            json->asObject()["current_tenant_id"] = Core::Json::makeString(tenantId);
        }
        if (!role.empty()) {
            json->asObject()["current_role"] = Core::Json::makeString(role);
        }

        LOG_DEBUG("✅ [USERS] Returning user data for: " + userId);
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        LOG_ERROR("❌ [USERS] Error in me(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*error);
    }
}

Response UsersController::getById(const Request& req) {
    try {
        std::string requestedUserId = req.getParam("id");
        std::string loggedUserId = req.getCustomData("user_id");

        LOG_DEBUG("🔵 [USERS] GET /api/users/:id - Requested: " + requestedUserId + ", Logged: " + loggedUserId);

        // Verificar se está tentando acessar outro usuário
        if (requestedUserId != loggedUserId) {
            LOG_WARNING("⚠️ [USERS] User " + loggedUserId + " tried to access user " + requestedUserId);
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("You can only view your own profile");
            return Response(StatusCode::Forbidden).json(*error);
        }

        auto userOpt = getUserUseCase_->execute(requestedUserId);

        if (!userOpt.has_value()) {
            LOG_ERROR("❌ [USERS] User not found: " + requestedUserId);
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("User not found");
            return Response(StatusCode::NotFound).json(*error);
        }

        auto user = userOpt.value();
        auto json = userToJson(user);

        LOG_DEBUG("✅ [USERS] Returning user: " + requestedUserId);
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        LOG_ERROR("❌ [USERS] Error in getById(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*error);
    }
}

Response UsersController::update(const Request& req) {
    try {
        std::string userId = req.getParam("id");
        std::string loggedUserId = req.getCustomData("user_id");

        LOG_DEBUG("🔵 [USERS] PUT /api/users/:id - User ID: " + userId);

        // Parse body
        auto jsonValue = req.getJson();
        if (!jsonValue || !jsonValue->isObject()) {
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*error);
        }

        auto obj = jsonValue->asObject();

        // Criar DTO
        UpdateUserDto dto;
        dto.userId = userId;
        dto.requestingUserId = loggedUserId;

        if (obj.count("name") && obj["name"]->isString()) {
            dto.name = obj["name"]->asString();
        }
        if (obj.count("email") && obj["email"]->isString()) {
            dto.email = obj["email"]->asString();
        }

        auto user = updateUserUseCase_->execute(dto);

        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("User updated successfully");
        json->asObject()["user"] = userToJson(user);

        LOG_DEBUG("✅ [USERS] User updated: " + userId);
        return Response(StatusCode::OK).json(*json);

    } catch (const std::invalid_argument& e) {
        LOG_ERROR("❌ [USERS] Invalid argument in update(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);

    } catch (const std::runtime_error& e) {
        LOG_ERROR("❌ [USERS] Runtime error in update(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Forbidden).json(*error);

    } catch (const std::exception& e) {
        LOG_ERROR("❌ [USERS] Error in update(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*error);
    }
}

Response UsersController::changePassword(const Request& req) {
    try {
        std::string userId = req.getParam("id");
        std::string loggedUserId = req.getCustomData("user_id");

        LOG_DEBUG("🔵 [USERS] PUT /api/users/:id/password - User ID: " + userId);

        // Parse body
        auto jsonValue = req.getJson();
        if (!jsonValue || !jsonValue->isObject()) {
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*error);
        }

        auto obj = jsonValue->asObject();

        // Aceita tanto snake_case quanto camelCase
        bool hasOldPassword = obj.count("old_password") || obj.count("oldPassword");
        bool hasNewPassword = obj.count("new_password") || obj.count("newPassword");

        if (!hasOldPassword || !hasNewPassword) {
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("old_password and new_password are required");
            return Response(StatusCode::BadRequest).json(*error);
        }

        std::string oldPassword = obj.count("old_password") ?
            obj["old_password"]->asString() : obj["oldPassword"]->asString();
        std::string newPassword = obj.count("new_password") ?
            obj["new_password"]->asString() : obj["newPassword"]->asString();

        ChangePasswordDto dto;
        dto.userId = userId;
        dto.requestingUserId = loggedUserId;
        dto.oldPassword = oldPassword;
        dto.newPassword = newPassword;

        changePasswordUseCase_->execute(dto);

        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("Password changed successfully");

        LOG_DEBUG("✅ [USERS] Password changed for user: " + userId);
        return Response(StatusCode::OK).json(*json);

    } catch (const std::invalid_argument& e) {
        LOG_ERROR("❌ [USERS] Invalid argument in changePassword(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);

    } catch (const std::runtime_error& e) {
        LOG_ERROR("❌ [USERS] Runtime error in changePassword(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Forbidden).json(*error);

    } catch (const std::exception& e) {
        LOG_ERROR("❌ [USERS] Error in changePassword(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*error);
    }
}

Response UsersController::changePasswordDeprecated(const Request& req) {
    try {
        std::string userId = req.getParam("id");
        std::string loggedUserId = req.getCustomData("user_id");

        LOG_DEBUG("🔵 [USERS] POST /api/users/:id/change-password (deprecated) - User ID: " + userId);

        // Parse body
        auto jsonValue = req.getJson();
        if (!jsonValue || !jsonValue->isObject()) {
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*error);
        }

        auto obj = jsonValue->asObject();

        if (!obj.count("oldPassword") || !obj.count("newPassword")) {
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("oldPassword and newPassword are required");
            return Response(StatusCode::BadRequest).json(*error);
        }

        ChangePasswordDto dto;
        dto.userId = userId;
        dto.requestingUserId = loggedUserId;
        dto.oldPassword = obj["oldPassword"]->asString();
        dto.newPassword = obj["newPassword"]->asString();

        changePasswordUseCase_->execute(dto);

        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("Password changed successfully");

        LOG_DEBUG("✅ [USERS] Password changed (deprecated endpoint) for user: " + userId);
        return Response(StatusCode::OK).json(*json);

    } catch (const std::invalid_argument& e) {
        LOG_ERROR("❌ [USERS] Invalid argument in changePasswordDeprecated(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);

    } catch (const std::runtime_error& e) {
        LOG_ERROR("❌ [USERS] Runtime error in changePasswordDeprecated(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Forbidden).json(*error);

    } catch (const std::exception& e) {
        LOG_ERROR("❌ [USERS] Error in changePasswordDeprecated(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*error);
    }
}

Response UsersController::delete_(const Request& req) {
    try {
        std::string userId = req.getParam("id");
        std::string loggedUserId = req.getCustomData("user_id");
        std::string loggedUserRole = req.getCustomData("user_role");

        LOG_DEBUG("🔵 [USERS] DELETE /api/users/:id - User ID: " + userId + ", Logged: " + loggedUserId);

        DeleteUserDto dto;
        dto.userId = userId;
        dto.requestingUserId = loggedUserId;
        dto.requestingUserRole = loggedUserRole;

        deleteUserUseCase_->execute(dto);

        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("User deleted successfully");

        LOG_DEBUG("✅ [USERS] User deleted: " + userId);
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        LOG_ERROR("❌ [USERS] Error in delete_(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Forbidden).json(*error);
    }
}

Response UsersController::list(const Request& req) {
    try {
        std::string loggedUserRole = req.getCustomData("user_role");

        LOG_DEBUG("🔵 [USERS] GET /api/users - Role: " + loggedUserRole);

        // Parse query params (limit, offset)
        // TODO: Implementar parsing de query params no Request
        int limit = 50;
        int offset = 0;

        ListUsersDto dto;
        dto.limit = limit;
        dto.offset = offset;
        dto.requestingUserRole = loggedUserRole;

        auto result = listUsersUseCase_->execute(dto);

        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["total"] = Core::Json::makeNumber(result.total);
        json->asObject()["limit"] = Core::Json::makeNumber(result.limit);
        json->asObject()["offset"] = Core::Json::makeNumber(result.offset);

        auto usersArray = Core::Json::makeArray();
        for (const auto& user : result.users) {
            usersArray->asArray().push_back(userToJson(user));
        }
        json->asObject()["users"] = usersArray;

        LOG_DEBUG("✅ [USERS] Returning " + std::to_string(result.users.size()) + " users");
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        LOG_ERROR("❌ [USERS] Error in list(): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*error);
    }
}

std::shared_ptr<Core::Json::JsonValue> UsersController::userToJson(const User& user) {
    auto json = Core::Json::makeObject();

    json->asObject()["id"] = Core::Json::makeString(user.getId());
    json->asObject()["email"] = Core::Json::makeString(user.getEmail().value());
    json->asObject()["name"] = Core::Json::makeString(user.getName());
    json->asObject()["is_active"] = Core::Json::makeBool(user.isActive());
    json->asObject()["email_verified"] = Core::Json::makeBool(user.isEmailVerified());

    // Não retornar password_hash por segurança!

    return json;
}

} // namespace Domains::UserManagement::Controllers

