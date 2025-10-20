/**
 * 🚀 C++ Backend do ZERO - VERSÃO 2.0
 * 
 * ✅ ConnectionPool - Pool de conexões reutilizáveis
 * ✅ Middleware System - Pipeline de requisições
 * ✅ Logger melhorado - Cores, debug mode, file output
 */

#include "../core/http/Server.hpp"
#include "../core/http/Router.hpp"
#include "../core/http/Request.hpp"
#include "../core/http/Response.hpp"
#include "../core/http/Middleware.hpp"
#include "../core/http/TenantMiddleware.hpp"
#include "../core/http/AuthMiddleware.hpp"
#include "../core/http/RoleMiddleware.hpp"
#include "../core/http/RateLimitMiddleware.hpp"
#include "../core/http/MetricsMiddleware.hpp"
#include "../core/utils/MetricsCollector.hpp"
#include "../core/database/Connection.hpp"
#include "../core/database/ConnectionPool.hpp"
#include "../core/utils/Logger.hpp"
#include "../core/utils/LoggerNew.hpp"
#include "../domains/identity/repositories/UserRepository.hpp"
#include "../domains/identity/repositories/TenantRepository.hpp"
#include "../domains/identity/services/JwtService.hpp"
#include "../domains/identity/services/BcryptService.hpp"
#include "../domains/identity/use_cases/RegisterUserUseCase.hpp"
#include "../domains/identity/use_cases/LoginUserUseCase.hpp"
#include "../domains/identity/use_cases/GetUserUseCase.hpp"
#include "../domains/identity/use_cases/UpdateUserUseCase.hpp"
#include "../domains/identity/use_cases/ChangePasswordUseCase.hpp"
#include "../domains/identity/use_cases/DeleteUserUseCase.hpp"
#include "../domains/identity/use_cases/ListUsersUseCase.hpp"
#include "../domains/identity/use_cases/ListTenantMembersUseCase.hpp"
#include "../domains/identity/use_cases/UpdateUserRoleUseCase.hpp"
#include "../domains/identity/use_cases/RemoveUserFromTenantUseCase.hpp"
#include "../domains/identity/use_cases/AddUserToTenantUseCase.hpp"
#include "../domains/identity/use_cases/RefreshTokenUseCase.hpp"
#include "../domains/identity/use_cases/SendVerificationEmailUseCase.hpp"
#include "../domains/identity/use_cases/VerifyEmailUseCase.hpp"
#include "../domains/identity/use_cases/RequestPasswordResetUseCase.hpp"
#include "../domains/identity/use_cases/ResetPasswordUseCase.hpp"
#include "../domains/identity/repositories/EmailVerificationRepository.hpp"
#include "../domains/identity/repositories/PasswordResetRepository.hpp"
#include "../domains/identity/repositories/AuditLogRepository.hpp"
#include "../domains/identity/services/EmailService.hpp"
#include "../domains/identity/controllers/AuthController.hpp"


#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>

using namespace Core::Http;
using namespace Core::Database;
using namespace Core::Utils;
using namespace Domains::Identity;

// ==================== GLOBALS ====================

std::unique_ptr<Server> globalServer;
std::shared_ptr<ConnectionPool> globalPool;
std::shared_ptr<Core::Utils::MetricsCollector> globalMetricsCollector;

/**
 * Signal handler para Ctrl+C
 */
void signalHandler(int signal) {
    std::cout << "\n🛑 Recebido sinal " << signal << ", parando servidor..." << std::endl;
    
    if (globalServer) {
        globalServer->stop();
    }
    
    if (globalPool) {
        globalPool->shutdown();
    }
    
    LoggerNew::shutdown();
    exit(0);
}

/**
 * Configurar rotas da aplicação
 */
void setupRoutes(Router& router, ConnectionPool& pool) {
    
    // ==================== HEALTH CHECK ====================
    router.get("/health", [](const Request& req) {
        (void)req;
        return Response(StatusCode::OK).text("OK - Server is running!");
    });
    
    // ==================== ROOT ====================
    router.get("/", [](const Request& req) {
        (void)req;
        std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>C++ Backend v2.0</title>
    <style>
        body {
            font-family: 'Segoe UI', Arial, sans-serif;
            max-width: 900px;
            margin: 50px auto;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .container {
            background: rgba(255,255,255,0.1);
            padding: 30px;
            border-radius: 10px;
            backdrop-filter: blur(10px);
        }
        h1 { font-size: 3em; margin: 0; }
        h2 { color: #ffd700; }
        .endpoint {
            background: rgba(0,0,0,0.2);
            padding: 15px;
            margin: 10px 0;
            border-radius: 5px;
            border-left: 4px solid #ffd700;
        }
        code {
            background: rgba(0,0,0,0.3);
            padding: 2px 8px;
            border-radius: 4px;
        }
        .new { color: #00ff88; font-weight: bold; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 C++ Backend v2.0</h1>
        <p><strong>100% C++ Puro - Agora com ConnectionPool e Middlewares!</strong></p>
        
        <h2>🆕 Novidades v2.0</h2>
        <ul>
            <li class="new">✅ Connection Pool (reutilização de conexões DB)</li>
            <li class="new">✅ Middleware System (pipeline de requisições)</li>
            <li class="new">✅ Logger melhorado (cores, debug mode, file output)</li>
            <li class="new">✅ CORS automático</li>
            <li class="new">✅ Request timing</li>
            <li class="new">✅ Error handling</li>
        </ul>
        
        <h2>📡 Endpoints</h2>
        
        <div class="endpoint">
            <strong>GET /health</strong><br>
            Health check simples
        </div>
        
        <div class="endpoint">
            <strong>GET /api/pool/stats</strong> <span class="new">NOVO</span><br>
            Estatísticas do connection pool
        </div>
        
        <div class="endpoint">
            <strong>GET /api/db/test</strong><br>
            Testa conexão com PostgreSQL (agora usando pool!)
        </div>
        
        <div class="endpoint">
            <strong>GET /api/db/query</strong> <span class="new">NOVO</span><br>
            Executa query de exemplo (múltiplas conexões)
        </div>
        
        <div class="endpoint">
            <strong>GET /api/stress/:connections</strong> <span class="new">NOVO</span><br>
            Teste de stress do pool (adquire N conexões simultâneas)
        </div>
        
        <h2>🧪 Experimente</h2>
        <p>Use curl ou seu navegador:</p>
        <code>curl http://localhost:8080/api/pool/stats</code><br>
        <code>curl http://localhost:8080/api/stress/5</code>
    </div>
</body>
</html>
)";
        return Response(StatusCode::OK).html(html);
    });
    
    

    auto userRepository = std::make_shared<Repositories::UserRepository>(globalPool);
    auto tenantRepository = std::make_shared<Repositories::TenantRepository>(globalPool);

// 1.5. Criar Services (JWT e Bcrypt)
auto bcryptService = std::make_shared<Services::BcryptService>(12); // cost factor 12
auto jwtService = std::make_shared<Services::JwtService>(
    "your-super-secret-jwt-key-change-this-in-production",  // Secret key
    60  // Expiration: 60 minutes
);

// 2. Criar Use Cases
auto registerUseCase = std::make_shared<UseCases::RegisterUserUseCase>(
    userRepository,
    bcryptService
);
auto loginUseCase = std::make_shared<UseCases::LoginUserUseCase>(
    userRepository,
    tenantRepository,
    bcryptService,
    jwtService
);
auto getUserUseCase = std::make_shared<UseCases::GetUserUseCase>(userRepository);

auto updateUserUseCase = std::make_shared<UseCases::UpdateUserUseCase>(userRepository);
auto changePasswordUseCase = std::make_shared<UseCases::ChangePasswordUseCase>(
    userRepository,
    bcryptService
);
auto deleteUserUseCase = std::make_shared<UseCases::DeleteUserUseCase>(userRepository);
auto listUsersUseCase = std::make_shared<UseCases::ListUsersUseCase>(userRepository);

// Tenant Management Use Cases
auto listTenantMembersUseCase = std::make_shared<UseCases::ListTenantMembersUseCase>(tenantRepository);
auto updateUserRoleUseCase = std::make_shared<UseCases::UpdateUserRoleUseCase>(tenantRepository);
auto removeUserFromTenantUseCase = std::make_shared<UseCases::RemoveUserFromTenantUseCase>(tenantRepository);
auto addUserToTenantUseCase = std::make_shared<UseCases::AddUserToTenantUseCase>(tenantRepository, userRepository);

// Refresh Token Use Case
auto refreshTokenUseCase = std::make_shared<UseCases::RefreshTokenUseCase>(jwtService, userRepository, tenantRepository);

// Email Verification & Password Reset
auto emailService = std::make_shared<Services::MockEmailService>();
auto emailVerificationRepository = std::make_shared<Repositories::EmailVerificationRepository>(globalPool);
auto passwordResetRepository = std::make_shared<Repositories::PasswordResetRepository>(globalPool);

// Audit Log Repository
auto auditLogRepository = std::make_shared<Repositories::AuditLogRepository>(globalPool);

auto sendVerificationEmailUseCase = std::make_shared<UseCases::SendVerificationEmailUseCase>(
    emailVerificationRepository,
    userRepository,
    emailService
);

auto verifyEmailUseCase = std::make_shared<UseCases::VerifyEmailUseCase>(
    emailVerificationRepository,
    userRepository,
    emailService
);

auto requestPasswordResetUseCase = std::make_shared<UseCases::RequestPasswordResetUseCase>(
    passwordResetRepository,
    userRepository,
    emailService
);

auto resetPasswordUseCase = std::make_shared<UseCases::ResetPasswordUseCase>(
    passwordResetRepository,
    userRepository,
    bcryptService
);

// 3. Criar Controller
auto authController = std::make_shared<Controllers::AuthController>(
    registerUseCase,
    loginUseCase
);

// 4. Configurar Rate Limiting
// Login: 5 tentativas por minuto (protege contra brute force)
auto rateLimitLogin = createRateLimitMiddleware(5, 60);

// Register: 3 tentativas por hora (previne spam de contas)
auto rateLimitRegister = createRateLimitMiddleware(3, 3600);

// Helper para aplicar rate limit a um handler
auto withRateLimit = [](MiddlewareFunction rateLimit, std::function<Response(const Request&)> handler) {
    return [rateLimit, handler](const Request& req) -> Response {
        Response res;
        bool rateLimitPassed = false;
        
        // Copiar request para poder modificar
        Request mutableReq = req;
        
        // Executar middleware de rate limit
        rateLimit(mutableReq, res, [&rateLimitPassed]() {
            rateLimitPassed = true;
        });
        
        // Se rate limit bloqueou, retornar resposta do middleware
        if (!rateLimitPassed) {
            return res;
        }
        
        // Rate limit OK, executar handler
        return handler(mutableReq);
    };
};

// 5. Registrar rotas com Rate Limiting
router.post("/api/auth/register", withRateLimit(rateLimitRegister, [authController](const Request& req) {
    return authController->register_(req);
}));

router.post("/api/auth/login", withRateLimit(rateLimitLogin, [authController](const Request& req) {
    return authController->login(req);
}));

// POST /api/auth/refresh - Renovar token
router.post("/api/auth/refresh", [refreshTokenUseCase](const Request& req) {
    // Parse body
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("refreshToken")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("refreshToken is required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    UseCases::RefreshTokenDto dto;
    dto.refreshToken = obj["refreshToken"]->asString();
    
    try {
        auto result = refreshTokenUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("Token refreshed successfully");
        json->asObject()["token"] = Core::Json::makeString(result.accessToken);
        json->asObject()["refreshToken"] = Core::Json::makeString(result.refreshToken);
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Unauthorized).json(*error);
    }
});

// ==================== EMAIL VERIFICATION ====================

// POST /api/auth/send-verification - Envia email de verificação
router.post("/api/auth/send-verification", [sendVerificationEmailUseCase](const Request& req) {
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("userId")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("userId is required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    UseCases::SendVerificationEmailDto dto;
    dto.userId = obj["userId"]->asString();
    
    try {
        sendVerificationEmailUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("Verification email sent successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
});

// POST /api/auth/verify-email - Verifica email com token
router.post("/api/auth/verify-email", [verifyEmailUseCase](const Request& req) {
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("token")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("token is required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    UseCases::VerifyEmailDto dto;
    dto.token = obj["token"]->asString();
    
    try {
        verifyEmailUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("Email verified successfully!");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
});

// ==================== PASSWORD RESET ====================

// POST /api/auth/forgot-password - Solicita reset de senha
router.post("/api/auth/forgot-password", [requestPasswordResetUseCase](const Request& req) {
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("email")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("email is required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    UseCases::RequestPasswordResetDto dto;
    dto.email = obj["email"]->asString();
    dto.ipAddress = "127.0.0.1"; // TODO: Extract from request
    dto.userAgent = req.getHeader("User-Agent");
    
    try {
        requestPasswordResetUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("If the email exists, a password reset link has been sent");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
});

// POST /api/auth/reset-password - Reseta senha com token
router.post("/api/auth/reset-password", [resetPasswordUseCase](const Request& req) {
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("token") || !obj.count("newPassword")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("token and newPassword are required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    UseCases::ResetPasswordDto dto;
    dto.token = obj["token"]->asString();
    dto.newPassword = obj["newPassword"]->asString();
    
    try {
        resetPasswordUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("Password reset successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
});

// ==================== ROTAS PROTEGIDAS ====================
// Criar AuthMiddleware
auto authMiddleware = createAuthMiddleware(jwtService);

// Helper para aplicar middleware a um handler
auto withAuth = [authMiddleware](std::function<Response(const Request&)> handler) {
    return [authMiddleware, handler](const Request& req) -> Response {
        Response res;
        bool authPassed = false;
        
        // Copiar request para poder modificar
        Request mutableReq = req;
        
        // Executar middleware
        authMiddleware(mutableReq, res, [&authPassed]() {
            authPassed = true;
        });
        
        // Se autenticação falhou, retornar resposta do middleware
        if (!authPassed) {
            return res;
        }
        
        // Autenticação OK, executar handler
        return handler(mutableReq);
    };
};

// Helper para aplicar role middleware a um handler
auto withRole = [](const std::vector<std::string>& allowedRoles, std::function<Response(const Request&)> handler) {
    auto roleMiddleware = createRoleMiddleware(allowedRoles);
    return [roleMiddleware, handler](const Request& req) -> Response {
        Response res;
        bool rolePassed = false;
        
        // Copiar request para poder modificar
        Request mutableReq = req;
        
        // Executar middleware de role
        roleMiddleware(mutableReq, res, [&rolePassed]() {
            rolePassed = true;
        });
        
        // Se verificação de role falhou, retornar resposta do middleware
        if (!rolePassed) {
            return res;
        }
        
        // Role OK, executar handler
        return handler(mutableReq);
    };
};

// Helper combinado: Auth + Role
auto withAuthAndRole = [&withAuth, &withRole](const std::vector<std::string>& allowedRoles, std::function<Response(const Request&)> handler) {
    return withAuth(withRole(allowedRoles, handler));
};

// GET /api/me - Retorna dados do usuário logado
router.get("/api/me", withAuth([getUserUseCase](const Request& req) {
    std::string userId = req.getCustomData("user_id");
    std::string userEmail = req.getCustomData("user_email");
    std::string tenantId = req.getCustomData("user_tenant_id");
    std::string role = req.getCustomData("user_role");
    
    auto userOpt = getUserUseCase->execute(userId);
    
    if (!userOpt.has_value()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("User not found");
        return Response(StatusCode::NotFound).json(*error);
    }
    
    auto user = userOpt.value();
    auto json = Core::Json::makeObject();
    json->asObject()["id"] = Core::Json::makeString(user.getId());
    json->asObject()["email"] = Core::Json::makeString(user.getEmail().value());
    json->asObject()["name"] = Core::Json::makeString(user.getName());
    json->asObject()["is_active"] = Core::Json::makeBool(user.isActive());
    json->asObject()["email_verified"] = Core::Json::makeBool(user.isEmailVerified());
    
    // Adicionar informações do token
    if (!tenantId.empty()) {
        json->asObject()["current_tenant_id"] = Core::Json::makeString(tenantId);
    }
    if (!role.empty()) {
        json->asObject()["current_role"] = Core::Json::makeString(role);
    }
    
    return Response(StatusCode::OK).json(*json);
}));

// GET /api/users/:id - Busca usuário (protegida)
router.get("/api/users/:id", withAuth([getUserUseCase](const Request& req) {
    std::string requestedUserId = req.getParam("id");
    std::string loggedUserId = req.getCustomData("user_id");
    
    // Verificar se está tentando acessar outro usuário
    if (requestedUserId != loggedUserId) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("You can only view your own profile");
        return Response(StatusCode::Forbidden).json(*error);
    }
    
    auto userOpt = getUserUseCase->execute(requestedUserId);
    
    if (!userOpt.has_value()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("User not found");
        return Response(StatusCode::NotFound).json(*error);
    }
    
    auto user = userOpt.value();
    auto json = Core::Json::makeObject();
    json->asObject()["id"] = Core::Json::makeString(user.getId());
    json->asObject()["email"] = Core::Json::makeString(user.getEmail().value());
    json->asObject()["name"] = Core::Json::makeString(user.getName());
    
    return Response(StatusCode::OK).json(*json);
}));

// PUT /api/users/:id - Atualizar usuário
router.put("/api/users/:id", withAuth([updateUserUseCase](const Request& req) {
    std::string userId = req.getParam("id");
    std::string loggedUserId = req.getCustomData("user_id");
    
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
    UseCases::UpdateUserDto dto;
    dto.userId = userId;
    dto.requestingUserId = loggedUserId;
    
    if (obj.count("name") && obj["name"]->isString()) {
        dto.name = obj["name"]->asString();
    }
    if (obj.count("email") && obj["email"]->isString()) {
        dto.email = obj["email"]->asString();
    }
    
    try {
        auto user = updateUserUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("User updated successfully");
        json->asObject()["user"] = Core::Json::makeObject();
        json->asObject()["user"]->asObject()["id"] = Core::Json::makeString(user.getId());
        json->asObject()["user"]->asObject()["email"] = Core::Json::makeString(user.getEmail().value());
        json->asObject()["user"]->asObject()["name"] = Core::Json::makeString(user.getName());
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
}));

// POST /api/users/:id/change-password - Trocar senha
router.post("/api/users/:id/change-password", withAuth([changePasswordUseCase](const Request& req) {
    std::string userId = req.getParam("id");
    std::string loggedUserId = req.getCustomData("user_id");
    
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
    
    UseCases::ChangePasswordDto dto;
    dto.userId = userId;
    dto.requestingUserId = loggedUserId;
    dto.oldPassword = obj["oldPassword"]->asString();
    dto.newPassword = obj["newPassword"]->asString();
    
    try {
        changePasswordUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("Password changed successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
}));

// DELETE /api/users/:id - Deletar usuário (soft delete)
router.del("/api/users/:id", withAuth([deleteUserUseCase](const Request& req) {
    std::string userId = req.getParam("id");
    std::string loggedUserId = req.getCustomData("user_id");
    std::string loggedUserRole = req.getCustomData("user_role");
    
    UseCases::DeleteUserDto dto;
    dto.userId = userId;
    dto.requestingUserId = loggedUserId;
    dto.requestingUserRole = loggedUserRole;
    
    try {
        deleteUserUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("User deleted successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Forbidden).json(*error);
    }
}));

// GET /api/users - Listar usuários (apenas admins)
router.get("/api/users", withAuthAndRole({"admin", "owner"}, [listUsersUseCase](const Request& req) {
    std::string loggedUserRole = req.getCustomData("user_role");
    
    // Parse query params (limit, offset)
    // TODO: Implementar parsing de query params no Request
    int limit = 50;
    int offset = 0;
    
    UseCases::ListUsersDto dto;
    dto.limit = limit;
    dto.offset = offset;
    dto.requestingUserRole = loggedUserRole;
    
    try {
        auto result = listUsersUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["total"] = Core::Json::makeNumber(result.total);
        json->asObject()["limit"] = Core::Json::makeNumber(result.limit);
        json->asObject()["offset"] = Core::Json::makeNumber(result.offset);
        
        auto usersArray = Core::Json::makeArray();
        for (const auto& user : result.users) {
            auto userObj = Core::Json::makeObject();
            userObj->asObject()["id"] = Core::Json::makeString(user.getId());
            userObj->asObject()["email"] = Core::Json::makeString(user.getEmail().value());
            userObj->asObject()["name"] = Core::Json::makeString(user.getName());
            userObj->asObject()["is_active"] = Core::Json::makeBool(user.isActive());
            usersArray->asArray().push_back(userObj);
        }
        json->asObject()["users"] = usersArray;
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Forbidden).json(*error);
    }
}));

// ==================== TENANT MANAGEMENT ====================

// GET /api/tenants/:id/members - Listar membros do tenant
router.get("/api/tenants/:tenantId/members", withAuth([listTenantMembersUseCase](const Request& req) {
    std::string tenantId = req.getParam("tenantId");
    std::string loggedUserId = req.getCustomData("user_id");
    std::string loggedUserRole = req.getCustomData("user_role");
    
    UseCases::ListTenantMembersDto dto;
    dto.tenantId = tenantId;
    dto.requestingUserId = loggedUserId;
    dto.requestingUserRole = loggedUserRole;
    
    try {
        auto members = listTenantMembersUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["total"] = Core::Json::makeNumber(members.size());
        
        auto membersArray = Core::Json::makeArray();
        for (const auto& member : members) {
            auto memberObj = Core::Json::makeObject();
            memberObj->asObject()["userId"] = Core::Json::makeString(member.userId);
            memberObj->asObject()["email"] = Core::Json::makeString(member.email);
            memberObj->asObject()["name"] = Core::Json::makeString(member.name);
            memberObj->asObject()["role"] = Core::Json::makeString(member.role);
            memberObj->asObject()["isActive"] = Core::Json::makeBool(member.isActive);
            membersArray->asArray().push_back(memberObj);
        }
        json->asObject()["members"] = membersArray;
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Forbidden).json(*error);
    }
}));

// POST /api/tenants/:id/users - Adicionar usuário ao tenant
router.post("/api/tenants/:tenantId/users", withAuthAndRole({"admin", "owner"}, [addUserToTenantUseCase](const Request& req) {
    std::string tenantId = req.getParam("tenantId");
    std::string loggedUserId = req.getCustomData("user_id");
    std::string loggedUserRole = req.getCustomData("user_role");
    
    // Parse body
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("userId") || !obj.count("role")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("userId and role are required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    UseCases::AddUserToTenantDto dto;
    dto.tenantId = tenantId;
    dto.userId = obj["userId"]->asString();
    dto.role = obj["role"]->asString();
    dto.requestingUserId = loggedUserId;
    dto.requestingUserRole = loggedUserRole;
    
    try {
        addUserToTenantUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("User added to tenant successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
}));

// PUT /api/tenants/:id/users/:userId/role - Atualizar role do usuário
router.put("/api/tenants/:tenantId/users/:userId/role", withAuthAndRole({"admin", "owner"}, [updateUserRoleUseCase](const Request& req) {
    std::string tenantId = req.getParam("tenantId");
    std::string userId = req.getParam("userId");
    std::string loggedUserId = req.getCustomData("user_id");
    std::string loggedUserRole = req.getCustomData("user_role");
    
    // Parse body
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("role")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("role is required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    UseCases::UpdateUserRoleDto dto;
    dto.tenantId = tenantId;
    dto.userId = userId;
    dto.newRole = obj["role"]->asString();
    dto.requestingUserId = loggedUserId;
    dto.requestingUserRole = loggedUserRole;
    
    try {
        updateUserRoleUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("User role updated successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
}));

// DELETE /api/tenants/:id/users/:userId - Remover usuário do tenant
router.del("/api/tenants/:tenantId/users/:userId", withAuthAndRole({"admin", "owner"}, [removeUserFromTenantUseCase](const Request& req) {
    std::string tenantId = req.getParam("tenantId");
    std::string userId = req.getParam("userId");
    std::string loggedUserId = req.getCustomData("user_id");
    std::string loggedUserRole = req.getCustomData("user_role");
    
    UseCases::RemoveUserFromTenantDto dto;
    dto.tenantId = tenantId;
    dto.userId = userId;
    dto.requestingUserId = loggedUserId;
    dto.requestingUserRole = loggedUserRole;
    
    try {
        removeUserFromTenantUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("User removed from tenant successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Forbidden).json(*error);
    }
}));

// ==================== AUDIT LOGS ====================

// GET /api/admin/audit-logs - Consultar logs de auditoria (admin only)
router.get("/api/admin/audit-logs", withAuthAndRole({"admin", "owner"}, [auditLogRepository](const Request& req) {
    std::string loggedUserId = req.getCustomData("user_id");
    std::string loggedUserRole = req.getCustomData("user_role");
    std::string tenantId = req.getCustomData("user_tenant_id");
    
    // TODO: Parse query params (user_id, tenant_id, action, limit, offset)
    // Por agora, buscar por tenant
    int limit = 50;
    int offset = 0;
    
    try {
        std::vector<Entities::AuditLog> logs;
        
        // Se tem tenant_id, buscar por tenant
        if (!tenantId.empty()) {
            logs = auditLogRepository->findByTenant(tenantId, limit, offset);
        } else {
            // Se não tem tenant, retornar vazio por segurança
            logs = {};
        }
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["total"] = Core::Json::makeNumber(logs.size());
        json->asObject()["limit"] = Core::Json::makeNumber(limit);
        json->asObject()["offset"] = Core::Json::makeNumber(offset);
        
        auto logsArray = Core::Json::makeArray();
        for (const auto& log : logs) {
            auto logObj = Core::Json::makeObject();
            logObj->asObject()["id"] = Core::Json::makeString(log.getId());
            
            if (log.getUserId().has_value()) {
                logObj->asObject()["user_id"] = Core::Json::makeString(log.getUserId().value());
            }
            
            if (log.getTenantId().has_value()) {
                logObj->asObject()["tenant_id"] = Core::Json::makeString(log.getTenantId().value());
            }
            
            logObj->asObject()["action"] = Core::Json::makeString(log.getAction());
            logObj->asObject()["resource"] = Core::Json::makeString(log.getResource());
            
            if (log.getResourceId().has_value()) {
                logObj->asObject()["resource_id"] = Core::Json::makeString(log.getResourceId().value());
            }
            
            logObj->asObject()["details"] = Core::Json::makeString(log.getDetails());
            logObj->asObject()["ip_address"] = Core::Json::makeString(log.getIpAddress());
            logObj->asObject()["user_agent"] = Core::Json::makeString(log.getUserAgent());
            
            logsArray->asArray().push_back(logObj);
        }
        json->asObject()["logs"] = logsArray;
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*error);
    }
}));

    // ==================== POOL STATS ====================
    router.get("/api/pool/stats", [&pool](const Request& req) {
        (void)req;
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["total_connections"] = Core::Json::makeNumber(pool.totalConnections());
        json->asObject()["available_connections"] = Core::Json::makeNumber(pool.availableConnections());
        json->asObject()["active_connections"] = Core::Json::makeNumber(pool.activeConnections());
        json->asObject()["pool_health"] = Core::Json::makeString(
            pool.availableConnections() > 0 ? "healthy" : "busy"
        );
        
        return Response(StatusCode::OK).json(*json);
    });
    
    // ==================== DB TEST (COM POOL) ====================
    router.get("/api/db/test", [&pool](const Request& req) {
        (void)req;
        
        auto json = Core::Json::makeObject();
        
        try {
            // Adquirir conexão do pool (RAII - devolve automaticamente)
            LOG_DEBUG("Adquirindo conexão do pool...");
            auto conn = pool.acquire();
            
            LOG_DEBUG("Executando query...");
            auto result = conn->execute("SELECT version(), current_database(), current_user");
            
            if (result.isSuccess() && result.rowCount() > 0) {
                json->asObject()["status"] = Core::Json::makeString("success");
                json->asObject()["connected"] = Core::Json::makeBool(true);
                json->asObject()["version"] = Core::Json::makeString(result.getValue(0, 0));
                json->asObject()["database"] = Core::Json::makeString(result.getValue(0, 1));
                json->asObject()["user"] = Core::Json::makeString(result.getValue(0, 2));
                json->asObject()["pool_stats"] = Core::Json::makeObject();
                json->asObject()["pool_stats"]->asObject()["total"] = Core::Json::makeNumber(pool.totalConnections());
                json->asObject()["pool_stats"]->asObject()["available"] = Core::Json::makeNumber(pool.availableConnections());
            } else {
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString(result.getError());
            }
            
            // Conexão devolvida automaticamente aqui!
            
        } catch (const std::exception& e) {
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString(e.what());
            return Response(StatusCode::InternalServerError).json(*json);
        }
        
        return Response(StatusCode::OK).json(*json);
    });
    
    // ==================== QUERY EXAMPLE ====================
    router.get("/api/db/query", [&pool](const Request& req) {
        (void)req;
        
        auto json = Core::Json::makeObject();
        
        try {
            auto conn = pool.acquire();
            
            // Exemplo: listar todas as tabelas do banco
            auto result = conn->execute(
                "SELECT table_name FROM information_schema.tables "
                "WHERE table_schema = 'public' "
                "ORDER BY table_name"
            );
            
            if (result.isSuccess()) {
                json->asObject()["status"] = Core::Json::makeString("success");
                json->asObject()["query"] = Core::Json::makeString("List public tables");
                json->asObject()["rows"] = Core::Json::makeNumber(result.rowCount());
                
                // Criar array de tabelas
                auto tablesArray = Core::Json::makeArray();
                for (int i = 0; i < result.rowCount() && i < 20; ++i) {
                    tablesArray->asArray().push_back(
                        Core::Json::makeString(result.getValue(i, 0))
                    );
                }
                json->asObject()["tables"] = tablesArray;
            } else {
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString(result.getError());
            }
            
        } catch (const std::exception& e) {
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString(e.what());
            return Response(StatusCode::InternalServerError).json(*json);
        }
        
        return Response(StatusCode::OK).json(*json);
    });
    
    // ==================== STRESS TEST ====================
    router.get("/api/stress/:connections", [&pool](const Request& req) {
        std::string connectionsStr = req.getParam("connections");
        int numConnections = std::stoi(connectionsStr);
        
        if (numConnections < 1 || numConnections > 20) {
            auto json = Core::Json::makeObject();
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString("Número de conexões deve estar entre 1 e 20");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto json = Core::Json::makeObject();
        
        try {
            LOG_INFO("🔥 Stress test: adquirindo " + std::to_string(numConnections) + " conexões...");
            
            // Adquirir múltiplas conexões
            std::vector<PooledConnection> connections;
            for (int i = 0; i < numConnections; ++i) {
                connections.push_back(pool.acquire());
                LOG_DEBUG("Conexão " + std::to_string(i+1) + " adquirida");
            }
            
            // Executar query em cada uma
            int successCount = 0;
            for (auto& conn : connections) {
                auto result = conn->execute("SELECT 1");
                if (result.isSuccess()) {
                    successCount++;
                }
            }
            
            json->asObject()["status"] = Core::Json::makeString("success");
            json->asObject()["connections_acquired"] = Core::Json::makeNumber(numConnections);
            json->asObject()["queries_executed"] = Core::Json::makeNumber(successCount);
            json->asObject()["pool_stats"] = Core::Json::makeObject();
            json->asObject()["pool_stats"]->asObject()["total"] = Core::Json::makeNumber(pool.totalConnections());
            json->asObject()["pool_stats"]->asObject()["available"] = Core::Json::makeNumber(pool.availableConnections());
            json->asObject()["pool_stats"]->asObject()["active"] = Core::Json::makeNumber(pool.activeConnections());
            
            // Conexões devolvidas automaticamente quando vector sai de escopo
            LOG_INFO("✅ Stress test completo!");
            
        } catch (const std::exception& e) {
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString(e.what());
            return Response(StatusCode::InternalServerError).json(*json);
        }
        
        return Response(StatusCode::OK).json(*json);
    });
}

/**
 * Entry point
 */
int main(int argc, char* argv[]) {
    try {
        // ==================== INICIALIZAR LOGGER ====================
        LoggerNew::init(
            "server.log",                    // Arquivo de log
            LoggerNew::Level::DEBUG,         // Nível mínimo
            true,                            // Usar cores
            true                             // Debug mode (mostrar arquivo:linha)
        );
        
        LOG_INFO("═══════════════════════════════════════");
        LOG_INFO("  🚀 C++ BACKEND v2.0");
        LOG_INFO("  ✅ ConnectionPool");
        LOG_INFO("  ✅ Middleware System");
        LOG_INFO("  ✅ Logger melhorado");
        LOG_INFO("═══════════════════════════════════════");
        
        // ==================== CRIAR CONNECTION POOL ====================
        LOG_INFO("Criando MetricsCollector...");
        globalMetricsCollector = std::make_shared<Core::Utils::MetricsCollector>();
        
        LOG_INFO("Criando connection pool...");
        
        std::string connectionString = 
            "host=localhost port=5433 dbname=moneymaker_dev user=moneymaker_user password=postgre123";
        
        globalPool = std::make_shared<ConnectionPool>(
            connectionString,
            2,    // Min: 2 conexões
            10,   // Max: 10 conexões
            5000  // Timeout: 5 segundos
        );
        
        LOG_INFO("✅ Connection pool criado!");
        
        // ==================== CRIAR REPOSITÓRIOS ====================
        auto tenantRepository = std::make_shared<Domains::Identity::Repositories::TenantRepository>(globalPool);
        LOG_DEBUG("✓ TenantRepository criado");
        
        // ==================== CRIAR SERVIDOR ====================
        int port = (argc > 1) ? std::atoi(argv[1]) : 8080;
        globalServer = std::make_unique<Server>(port);
        
        // ==================== CONFIGURAR MIDDLEWARES ====================
        LOG_INFO("Configurando middlewares...");
        
        auto middlewares = std::make_shared<MiddlewareChain>();
        
        // 1. CORS (todas as rotas)
        middlewares->use(Middlewares::cors());
        LOG_DEBUG("✓ CORS middleware");
        
        // 2. Tenant Resolution (extrai tenant do subdomain)
        middlewares->use(createTenantMiddleware(tenantRepository));
        LOG_DEBUG("✓ Tenant middleware");
        
        // 2.5. Metrics Collection (coleta métricas de todas as rotas)
        middlewares->use(createMetricsMiddleware(globalMetricsCollector));
        LOG_DEBUG("✓ Metrics middleware");
        
        // 3. Request Logger (todas as rotas)
        middlewares->use(Middlewares::requestLogger());
        LOG_DEBUG("✓ Request logger middleware");
        
        // 3. Timing (todas as rotas)
        middlewares->use(Middlewares::timing());
        LOG_DEBUG("✓ Timing middleware");
        
        // 4. Error Handler (todas as rotas)
        middlewares->use(Middlewares::errorHandler());
        LOG_DEBUG("✓ Error handler middleware");
        
        LOG_INFO("✅ " + std::to_string(middlewares->size()) + " middlewares configurados!");
        
        // ==================== CONFIGURAR ROUTER ====================
        auto router = std::make_shared<Router>();
        router->setMiddlewares(middlewares);  // Integrar middlewares
        setupRoutes(*router, *globalPool);
        
        // ==================== METRICS ENDPOINT ====================
        // Adicionar DEPOIS de setupRoutes para ter acesso a globalMetricsCollector
        router->get("/metrics", [](const Request& req) {
            (void)req;
            
            try {
                std::string prometheusOutput = globalMetricsCollector->exportPrometheus();
                
                Response response(StatusCode::OK);
                response.setHeader("Content-Type", "text/plain; version=0.0.4; charset=utf-8");
                response.setBody(prometheusOutput);
                return response;
                    
            } catch (const std::exception& e) {
                std::string error = "# ERROR: " + std::string(e.what()) + "\n";
                Response response(StatusCode::InternalServerError);
                response.setHeader("Content-Type", "text/plain");
                response.setBody(error);
                return response;
            }
        });
        
        globalServer->setRouter(router);
        
        // ==================== SIGNAL HANDLER ====================
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        
        // ==================== INICIAR SERVIDOR ====================
        LOG_INFO("🚀 Servidor iniciando na porta " + std::to_string(port) + "...");
        globalServer->start();
        
    } catch (const std::exception& e) {
        LOG_FATAL(std::string("Erro fatal: ") + e.what());
        return 1;
    }
    
    return 0;
}

