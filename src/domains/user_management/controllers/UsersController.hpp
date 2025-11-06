#pragma once

#include "../use_cases/GetUserUseCase.hpp"
#include "../use_cases/UpdateUserUseCase.hpp"
#include "../use_cases/ChangePasswordUseCase.hpp"
#include "../use_cases/DeleteUserUseCase.hpp"
#include "../use_cases/ListUsersUseCase.hpp"
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include <memory>

namespace Domains::UserManagement::Controllers {

using Domains::UserManagement::Entities::User;
using namespace Domains::UserManagement::UseCases;
using namespace Core::Http;

/**
 * @brief Controller para gerenciamento de usuários.
 *
 * Rotas:
 * - GET    /api/me - Retorna dados do usuário logado
 * - GET    /api/users/:id - Busca usuário por ID
 * - PUT    /api/users/:id - Atualizar usuário
 * - PUT    /api/users/:id/password - Trocar senha (snake_case)
 * - POST   /api/users/:id/change-password - Trocar senha (camelCase, deprecated)
 * - DELETE /api/users/:id - Deletar usuário (soft delete)
 * - GET    /api/users - Listar usuários (apenas admins)
 */
class UsersController {
public:
    UsersController(
        std::shared_ptr<GetUserUseCase> getUserUseCase,
        std::shared_ptr<UpdateUserUseCase> updateUserUseCase,
        std::shared_ptr<ChangePasswordUseCase> changePasswordUseCase,
        std::shared_ptr<DeleteUserUseCase> deleteUserUseCase,
        std::shared_ptr<ListUsersUseCase> listUsersUseCase
    );

    /**
     * @brief GET /api/me - Retorna dados do usuário logado
     *
     * Headers: Authorization: Bearer <token>
     */
    Response me(const Request& req);

    /**
     * @brief GET /api/users/:id - Busca usuário por ID
     *
     * Headers: Authorization: Bearer <token>
     */
    Response getById(const Request& req);

    /**
     * @brief PUT /api/users/:id - Atualizar usuário
     *
     * Headers: Authorization: Bearer <token>
     * Body: { "name": "...", "email": "..." }
     */
    Response update(const Request& req);

    /**
     * @brief PUT /api/users/:id/password - Trocar senha (snake_case)
     *
     * Headers: Authorization: Bearer <token>
     * Body: { "old_password": "...", "new_password": "..." }
     */
    Response changePassword(const Request& req);

    /**
     * @brief POST /api/users/:id/change-password - Trocar senha (camelCase, deprecated)
     *
     * Headers: Authorization: Bearer <token>
     * Body: { "oldPassword": "...", "newPassword": "..." }
     */
    Response changePasswordDeprecated(const Request& req);

    /**
     * @brief DELETE /api/users/:id - Deletar usuário (soft delete)
     *
     * Headers: Authorization: Bearer <token>
     */
    Response delete_(const Request& req);

    /**
     * @brief GET /api/users - Listar usuários (apenas admins)
     *
     * Headers: Authorization: Bearer <token>
     * Query params: limit, offset
     */
    Response list(const Request& req);

private:
    std::shared_ptr<GetUserUseCase> getUserUseCase_;
    std::shared_ptr<UpdateUserUseCase> updateUserUseCase_;
    std::shared_ptr<ChangePasswordUseCase> changePasswordUseCase_;
    std::shared_ptr<DeleteUserUseCase> deleteUserUseCase_;
    std::shared_ptr<ListUsersUseCase> listUsersUseCase_;
    
    // Helper para serializar User para JSON
    std::shared_ptr<Core::Json::JsonValue> userToJson(const User& user);
};

} // namespace Domains::UserManagement::Controllers
