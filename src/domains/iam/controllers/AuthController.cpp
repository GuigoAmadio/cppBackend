#include "AuthController.hpp"
#include "../../../core/json/Json.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::IAM::Controllers {

AuthController::AuthController(
    std::shared_ptr<RegisterUserUseCase> registerUseCase,
    std::shared_ptr<LoginUserUseCase> loginUseCase
) : registerUseCase_(registerUseCase),
    loginUseCase_(loginUseCase) {}

Response AuthController::register_(const Request& req) {
    try {
        // DEBUG: Log body
        LOG_DEBUG("Body received: " + req.getBody());
        LOG_DEBUG("Content-Type: " + req.getHeader("Content-Type"));
        
        // 1. Parse JSON body
        auto json = req.getJson();
        
        if (!json) {
            LOG_ERROR("getJson() returned nullptr");
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("Failed to parse JSON - body: " + req.getBody());
            return Response(StatusCode::BadRequest).json(*error);
        }
        
        if (!json->isObject()) {
            LOG_ERROR("JSON is not an object");
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("JSON body must be an object");
            return Response(StatusCode::BadRequest).json(*error);
        }
        
        auto& obj = json->asObject();
        
        // 2. Extrair campos
        if (!obj.count("email") || !obj.count("password") || !obj.count("name")) {
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("Missing required fields: email, password, name");
            return Response(StatusCode::BadRequest).json(*error);
        }
        
        RegisterUserDto dto;
        dto.email = obj["email"]->asString();
        dto.password = obj["password"]->asString();
        dto.name = obj["name"]->asString();
        
        // tenant_subdomain é opcional: se não fornecido, cria novo tenant
        if (obj.count("tenant_subdomain") && obj["tenant_subdomain"]->isString()) {
            dto.tenant_subdomain = obj["tenant_subdomain"]->asString();
        }
        // Note: Role is now tenant-specific and assigned in user_tenants table
        
        // 3. Executar use case
        User user = registerUseCase_->execute(dto);
        
        // 4. Retornar resposta
        auto response = Core::Json::makeObject();
        response->asObject()["status"] = Core::Json::makeString("success");
        response->asObject()["message"] = Core::Json::makeString("User registered successfully");
        response->asObject()["user"] = userToJson(user);
        
        return Response(StatusCode::Created).json(*response);
        
    } catch (const std::invalid_argument& e) {
        // Erro de validação
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
        
    } catch (const std::runtime_error& e) {
        // Erro de negócio (ex: email já existe, tenant não encontrado)
        LOG_WARNING("Register failed (runtime_error): " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Conflict).json(*error);
        
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Register error: ") + e.what());
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*error);
    }
}

Response AuthController::login(const Request& req) {
    try {
        // DEBUG: Log body
        LOG_DEBUG("Login - Body received: " + req.getBody());
        LOG_DEBUG("Login - Content-Type: " + req.getHeader("Content-Type"));
        
        // 1. Parse JSON body
        auto json = req.getJson();
        
        if (!json) {
            LOG_ERROR("Login - getJson() returned nullptr");
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("Failed to parse JSON - body: " + req.getBody());
            return Response(StatusCode::BadRequest).json(*error);
        }
        
        if (!json->isObject()) {
            LOG_ERROR("Login - JSON is not an object");
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("JSON body must be an object");
            return Response(StatusCode::BadRequest).json(*error);
        }
        
        auto& obj = json->asObject();
        
        // 2. Extrair campos
        if (!obj.count("email") || !obj.count("password")) {
            auto error = Core::Json::makeObject();
            error->asObject()["status"] = Core::Json::makeString("error");
            error->asObject()["message"] = Core::Json::makeString("Missing required fields: email, password");
            return Response(StatusCode::BadRequest).json(*error);
        }
        
        LoginUserDto dto;
        dto.email = obj["email"]->asString();
        dto.password = obj["password"]->asString();
        
        // Extrair tenant context - PRIORIDADE: body > middleware
        // 1. Tentar do body (enviado explicitamente pelo cliente)
        if (obj.count("tenant_subdomain") && obj["tenant_subdomain"]->isString()) {
            dto.tenant_subdomain = obj["tenant_subdomain"]->asString();
            LOG_DEBUG("tenant_subdomain from body: " + dto.tenant_subdomain);
        } else {
            // 2. Fallback para middleware (extraído do subdomain da URL)
            dto.tenant_subdomain = req.getCustomData("tenant_subdomain");
        }
        
        // tenant_id sempre do middleware (setado pelo TenantMiddleware após resolver subdomain)
        dto.tenant_id = req.getCustomData("tenant_id");
        
        LOG_DEBUG("Login attempt: email=" + dto.email + ", tenant_id=" + dto.tenant_id + 
                  ", tenant_subdomain=" + dto.tenant_subdomain);
        
        // 3. Executar use case
        LoginResult result = loginUseCase_->execute(dto);
        
        // 4. Retornar resposta
        auto response = Core::Json::makeObject();
        response->asObject()["status"] = Core::Json::makeString("success");
        response->asObject()["message"] = Core::Json::makeString("Login successful");
        response->asObject()["user"] = userToJson(result.user);
        response->asObject()["token"] = Core::Json::makeString(result.token);
        response->asObject()["refreshToken"] = Core::Json::makeString(result.refreshToken);
        
        return Response(StatusCode::OK).json(*response);
        
    } catch (const std::runtime_error& e) {
        // Erro de autenticação
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Unauthorized).json(*error);
        
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Login error: ") + e.what());
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*error);
    }
}

Response AuthController::me(const Request& req) {
    (void)req;
    // TODO: Implementar quando JWT estiver pronto
    auto error = Core::Json::makeObject();
    error->asObject()["status"] = Core::Json::makeString("error");
    error->asObject()["message"] = Core::Json::makeString("Not implemented yet - JWT required");
    return Response(StatusCode::NotImplemented).json(*error);
}

std::shared_ptr<Core::Json::JsonValue> AuthController::userToJson(const User& user) {
    auto json = Core::Json::makeObject();
    
    json->asObject()["id"] = Core::Json::makeString(user.getId());
    json->asObject()["email"] = Core::Json::makeString(user.getEmail().value());
    json->asObject()["name"] = Core::Json::makeString(user.getName());
    // Note: Role is now tenant-specific and should be included from user_tenants if needed
    json->asObject()["is_active"] = Core::Json::makeBool(user.isActive());
    json->asObject()["email_verified"] = Core::Json::makeBool(user.isEmailVerified());
    
    // Não retornar password_hash por segurança!
    
    return json;
}

} // namespace Domains::IAM::Controllers

