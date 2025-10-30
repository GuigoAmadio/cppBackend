#include "LoginUserUseCase.hpp"
#include "../../audit/entities/AuditLog.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::IAM::UseCases {

LoginUserUseCase::LoginUserUseCase(
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> repository,
    std::shared_ptr<Domains::TenantManagement::Repositories::TenantRepository> tenantRepository,
    std::shared_ptr<Domains::IAM::Services::BcryptService> bcryptService,
    std::shared_ptr<Domains::IAM::Services::JwtService> jwtService,
    std::shared_ptr<Domains::Audit::Repositories::AuditLogRepository> auditLogRepository
) : repository_(repository), 
    tenantRepository_(tenantRepository),
    bcryptService_(bcryptService),
    jwtService_(jwtService),
    auditLogRepository_(auditLogRepository) {}

LoginResult LoginUserUseCase::execute(const LoginUserDto& dto) {
    // 1. Buscar usuário por email
    auto userOpt = repository_->findByEmail(dto.email);
    
    if (!userOpt.has_value()) {
        LOG_WARNING("Login failed: user not found - " + dto.email);
        throw std::runtime_error("Invalid credentials");
    }
    
    User user = userOpt.value();
    
    // 2. Verificar se usuário está ativo
    if (!user.isActive()) {
        LOG_WARNING("Login failed: user inactive - " + dto.email);
        throw std::runtime_error("User account is inactive");
    }
    
    // 3. Verificar senha usando bcrypt
    if (!bcryptService_->verify(dto.password, user.getPasswordHash())) {
        LOG_WARNING("Login failed: wrong password - " + dto.email);
        throw std::runtime_error("Invalid credentials");
    }
    
    // 4. Resolver tenant_subdomain para tenant_id (se necessário)
    LoginUserDto resolvedDto = dto; // Criar cópia mutável
    
    if (resolvedDto.tenant_id.empty() && !resolvedDto.tenant_subdomain.empty()) {
        // Cliente forneceu tenant_subdomain mas não tenant_id - precisamos resolver
        LOG_DEBUG("Resolving tenant_subdomain '" + resolvedDto.tenant_subdomain + "' to tenant_id");
        
        auto tenantOpt = tenantRepository_->findBySubdomain(resolvedDto.tenant_subdomain);
        if (!tenantOpt.has_value()) {
            LOG_WARNING("Login failed: tenant not found - " + resolvedDto.tenant_subdomain);
            throw std::runtime_error("Tenant not found: " + resolvedDto.tenant_subdomain);
        }
        
        resolvedDto.tenant_id = tenantOpt->getId();
        LOG_DEBUG("Resolved tenant_subdomain '" + resolvedDto.tenant_subdomain + "' to tenant_id: " + resolvedDto.tenant_id);
    }
    
    // 5. Verificar se tenant foi fornecido e buscar role
    std::string role = "user"; // default
    
    if (!resolvedDto.tenant_id.empty()) {
        LOG_DEBUG("🔍 [LOGIN] Checking user access to tenant - user_id: " + user.getId() + ", tenant_id: " + resolvedDto.tenant_id);
        
        // Verificar se usuário tem acesso a este tenant
        bool hasAccess = tenantRepository_->userHasAccessToTenant(user.getId(), resolvedDto.tenant_id);
        LOG_DEBUG("🔍 [LOGIN] userHasAccessToTenant returned: " + std::string(hasAccess ? "TRUE" : "FALSE"));
        
        if (!hasAccess) {
            LOG_WARNING("❌ [LOGIN] Login failed: user has no access to tenant - " + dto.email + " @ " + resolvedDto.tenant_subdomain);
            LOG_WARNING("   User ID: " + user.getId());
            LOG_WARNING("   Tenant ID: " + resolvedDto.tenant_id);
            throw std::runtime_error("No access to this tenant");
        }
        
        LOG_DEBUG("✅ [LOGIN] User has access to tenant, fetching role...");
        
        // Buscar role do usuário neste tenant
        auto roleOpt = tenantRepository_->getUserRoleInTenant(user.getId(), resolvedDto.tenant_id);
        if (roleOpt.has_value()) {
            role = roleOpt.value();
            LOG_DEBUG("✅ [LOGIN] User role in tenant: " + role);
        } else {
            LOG_DEBUG("⚠️  [LOGIN] No role found for user in tenant, using default: " + role);
        }
    } else {
        LOG_DEBUG("ℹ️  [LOGIN] No tenant_id provided, login without tenant context");
    }
    
    // 6. Registrar login
    user.recordLogin();
    repository_->update(user);
    
    // 7. Gerar tokens JWT com contexto de tenant (usando dto resolvido)
    std::string token;
    
    if (!resolvedDto.tenant_id.empty()) {
        // Token com tenant context
        token = jwtService_->generateTokenForTenant(
            user.getId(),
            user.getEmail().value(),
            resolvedDto.tenant_id,
            resolvedDto.tenant_subdomain,
            role
        );
        LOG_INFO("User logged in with tenant context: " + user.getEmail().value() + 
                 " @ " + resolvedDto.tenant_subdomain + " (tenant_id: " + resolvedDto.tenant_id + ", role: " + role + ")");
    } else {
        // Fallback: token sem tenant (para compatibilidade)
        token = jwtService_->generateToken(
            user.getId(),
            user.getEmail().value(),
            role
        );
        LOG_INFO("User logged in without tenant: " + user.getEmail().value());
    }
    
    std::string refreshToken = jwtService_->generateRefreshToken(user.getId());
    
    // 7. Registrar no Audit Log
    try {
        std::string detailsJson = "{\"tenant_subdomain\": \"" + 
            (dto.tenant_subdomain.empty() ? "none" : dto.tenant_subdomain) + "\"}";
        
        auditLogRepository_->log(
            user.getId(),                           // user_id
            dto.tenant_id.empty() ? std::nullopt : std::optional<std::string>(dto.tenant_id), // tenant_id
            "USER_LOGIN",                           // action
            "auth",                                 // resource
            user.getId(),                           // resource_id
            detailsJson,                            // details (JSON)
            "127.0.0.1",                           // ip_address (TODO: get real IP)
            "UserAgent/1.0"                        // user_agent (TODO: get real User-Agent)
        );
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create audit log: " + std::string(e.what()));
        // Não falhar o login por causa de erro no audit log
    }
    
    return LoginResult{user, token, refreshToken};
}

} // namespace Domains::IAM::UseCases

