#pragma once

#include "../use_cases/RegisterUserUseCase.hpp"
#include "../use_cases/LoginUserUseCase.hpp"
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include <memory>

namespace Domains::Identity::Controllers {

using namespace UseCases;
using namespace Core::Http;

/**
 * @brief Controller para autenticação.
 * 
 * Rotas:
 * - POST /api/auth/register
 * - POST /api/auth/login
 * - GET  /api/auth/me (TODO: requer JWT)
 */
class AuthController {
public:
    AuthController(
        std::shared_ptr<RegisterUserUseCase> registerUseCase,
        std::shared_ptr<LoginUserUseCase> loginUseCase
    );
    
    /**
     * @brief POST /api/auth/register
     * 
     * Body: { "email": "...", "password": "...", "name": "..." }
     */
    Response register_(const Request& req);
    
    /**
     * @brief POST /api/auth/login
     * 
     * Body: { "email": "...", "password": "..." }
     */
    Response login(const Request& req);
    
    /**
     * @brief GET /api/auth/me
     * 
     * Headers: Authorization: Bearer <token>
     * TODO: Implementar quando JWT estiver pronto
     */
    Response me(const Request& req);

private:
    std::shared_ptr<RegisterUserUseCase> registerUseCase_;
    std::shared_ptr<LoginUserUseCase> loginUseCase_;
    
    // Helper para serializar User para JSON
    std::shared_ptr<Core::Json::JsonValue> userToJson(const User& user);
};

} // namespace Domains::Identity::Controllers

