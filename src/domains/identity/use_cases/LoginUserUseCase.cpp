#include "LoginUserUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Identity::UseCases {

LoginUserUseCase::LoginUserUseCase(
    std::shared_ptr<UserRepository> repository,
    std::shared_ptr<TenantRepository> tenantRepository,
    std::shared_ptr<Services::BcryptService> bcryptService,
    std::shared_ptr<Services::JwtService> jwtService
) : repository_(repository), 
    tenantRepository_(tenantRepository),
    bcryptService_(bcryptService),
    jwtService_(jwtService) {}

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
    
    // 4. Verificar se tenant foi fornecido e buscar role
    std::string role = "user"; // default
    
    if (!dto.tenant_id.empty()) {
        // Verificar se usuário tem acesso a este tenant
        if (!tenantRepository_->userHasAccessToTenant(user.getId(), dto.tenant_id)) {
            LOG_WARNING("Login failed: user has no access to tenant - " + dto.email + " @ " + dto.tenant_subdomain);
            throw std::runtime_error("No access to this tenant");
        }
        
        // Buscar role do usuário neste tenant
        auto roleOpt = tenantRepository_->getUserRoleInTenant(user.getId(), dto.tenant_id);
        if (roleOpt.has_value()) {
            role = roleOpt.value();
        }
    }
    
    // 5. Registrar login
    user.recordLogin();
    repository_->update(user);
    
    // 6. Gerar tokens JWT com contexto de tenant
    std::string token;
    
    if (!dto.tenant_id.empty()) {
        // Token com tenant context
        token = jwtService_->generateTokenForTenant(
            user.getId(),
            user.getEmail().value(),
            dto.tenant_id,
            dto.tenant_subdomain,
            role
        );
        LOG_INFO("User logged in with tenant context: " + user.getEmail().value() + 
                 " @ " + dto.tenant_subdomain + " (role: " + role + ")");
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
    
    return LoginResult{user, token, refreshToken};
}

} // namespace Domains::Identity::UseCases

