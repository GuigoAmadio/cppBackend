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
        LOG_DEBUG("🔵 [REGISTER] Calling use case with email: " + dto.email);
        RegisterResult result = registerUseCase_->execute(dto);
        LOG_DEBUG("✅ [REGISTER] Use case executed successfully");
        LOG_DEBUG("   User ID: " + result.user.getId());
        LOG_DEBUG("   Tenant ID: " + result.tenant_id);
        LOG_DEBUG("   Tenant Subdomain: " + result.tenant_subdomain);
        LOG_DEBUG("   Role: " + result.role);
        LOG_DEBUG("   Token generated: " + std::string(result.token.empty() ? "NO" : "YES"));
        LOG_DEBUG("   RefreshToken generated: " + std::string(result.refreshToken.empty() ? "NO" : "YES"));
        
        // 4. Retornar resposta com tokens
        auto response = Core::Json::makeObject();
        response->asObject()["status"] = Core::Json::makeString("success");
        response->asObject()["message"] = Core::Json::makeString("User registered successfully");
        response->asObject()["user"] = userToJson(result.user);
        response->asObject()["tokens"] = Core::Json::makeObject();
        response->asObject()["tokens"]->asObject()["accessToken"] = Core::Json::makeString(result.token);
        response->asObject()["tokens"]->asObject()["refreshToken"] = Core::Json::makeString(result.refreshToken);
        response->asObject()["tenant"] = Core::Json::makeObject();
        response->asObject()["tenant"]->asObject()["id"] = Core::Json::makeString(result.tenant_id);
        response->asObject()["tenant"]->asObject()["subdomain"] = Core::Json::makeString(result.tenant_subdomain);
        response->asObject()["role"] = Core::Json::makeString(result.role);
        
        LOG_DEBUG("✅ [REGISTER] Returning successful response with tokens");
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
        
        // Extrair tenant context com lógica de prioridade baseada em role
        // 1. Verificar se há token válido (usuário já autenticado tentando acessar outro tenant)
        std::string userRole = req.getCustomData("user_role");
        std::string tokenTenantSubdomain = req.getCustomData("user_tenant_subdomain");
        std::string bodyTenantSubdomain = "";
        std::string middlewareTenantSubdomain = req.getCustomData("tenant_subdomain");
        
        // Extrair tenant_subdomain do body se fornecido
        if (obj.count("tenant_subdomain") && obj["tenant_subdomain"]->isString()) {
            bodyTenantSubdomain = obj["tenant_subdomain"]->asString();
            LOG_DEBUG("tenant_subdomain from body: " + bodyTenantSubdomain);
        }
        
        // Lógica de prioridade:
        // - Se usuário tem role "super_admin" no token E forneceu tenant_subdomain no body:
        //   → Usar tenant do body (permite acessar outro tenant)
        // - Caso contrário:
        //   → Usar tenant do token (se existir) ou do middleware
        
        if (userRole == "super_admin" && !bodyTenantSubdomain.empty()) {
            // Super admin pode usar tenant do body para acessar outro tenant
            dto.tenant_subdomain = bodyTenantSubdomain;
            LOG_DEBUG("Super admin detected - using tenant_subdomain from body: " + bodyTenantSubdomain);
        } else if (!tokenTenantSubdomain.empty()) {
            // Usar tenant do token (usuário já autenticado)
            dto.tenant_subdomain = tokenTenantSubdomain;
            LOG_DEBUG("Using tenant_subdomain from token: " + tokenTenantSubdomain);
        } else if (!bodyTenantSubdomain.empty()) {
            // Login sem token anterior - usar do body
            dto.tenant_subdomain = bodyTenantSubdomain;
            LOG_DEBUG("Using tenant_subdomain from body (no token): " + bodyTenantSubdomain);
        } else {
            // Fallback para middleware (extraído do subdomain da URL)
            dto.tenant_subdomain = middlewareTenantSubdomain;
            LOG_DEBUG("Using tenant_subdomain from middleware: " + middlewareTenantSubdomain);
        }
        
        // tenant_id sempre do middleware (setado pelo TenantMiddleware após resolver subdomain)
        // Se tenant_subdomain veio do body/token, o tenant_id será resolvido no use case
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

