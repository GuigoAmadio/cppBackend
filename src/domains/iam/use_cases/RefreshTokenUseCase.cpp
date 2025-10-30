#include "RefreshTokenUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::IAM::UseCases {

RefreshTokenUseCase::RefreshTokenUseCase(
    std::shared_ptr<JwtService> jwtService,
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> userRepository,
    std::shared_ptr<Domains::TenantManagement::Repositories::TenantRepository> tenantRepository
) : jwtService_(jwtService), 
    userRepository_(userRepository),
    tenantRepository_(tenantRepository) {}

RefreshTokenResult RefreshTokenUseCase::execute(const RefreshTokenDto& dto) {
    // 1. Validar refresh token
    auto claims = jwtService_->validateToken(dto.refreshToken);
    
    if (claims.empty()) {
        LOG_WARNING("RefreshToken failed: invalid token");
        throw std::runtime_error("Invalid refresh token");
    }
    
    // 2. Verificar se é um refresh token (tem claim "type": "refresh")
    if (claims.count("type") == 0 || claims["type"] != "refresh") {
        LOG_WARNING("RefreshToken failed: not a refresh token");
        throw std::runtime_error("Token is not a refresh token");
    }
    
    // 3. Verificar se token não expirou
    if (jwtService_->isTokenExpired(dto.refreshToken)) {
        LOG_WARNING("RefreshToken failed: token expired");
        throw std::runtime_error("Refresh token expired");
    }
    
    // 4. Extrair user_id
    if (claims.count("sub") == 0) {
        LOG_ERROR("RefreshToken failed: missing 'sub' claim");
        throw std::runtime_error("Invalid token structure");
    }
    
    std::string userId = claims["sub"];
    
    // 5. Verificar se usuário existe e está ativo
    auto userOpt = userRepository_->findById(userId);
    
    if (!userOpt.has_value()) {
        LOG_WARNING("RefreshToken failed: user not found - " + userId);
        throw std::runtime_error("User not found");
    }
    
    auto user = userOpt.value();
    
    if (!user.isActive()) {
        LOG_WARNING("RefreshToken failed: user is inactive - " + userId);
        throw std::runtime_error("User account is inactive");
    }
    
    // 6. Extrair contexto de tenant se presente no token original
    std::string tenantId = claims.count("tenant_id") ? claims["tenant_id"] : "";
    std::string tenantSubdomain = claims.count("tenant_subdomain") ? claims["tenant_subdomain"] : "";
    std::string role = "user"; // Default
    
    // 7. Se tem tenant context, buscar role atualizada
    if (!tenantId.empty()) {
        // Verificar se usuário ainda tem acesso ao tenant
        if (!tenantRepository_->userHasAccessToTenant(userId, tenantId)) {
            LOG_WARNING("RefreshToken: user lost access to tenant - " + userId + " @ " + tenantId);
            // Não bloqueia, mas remove tenant context
            tenantId = "";
            tenantSubdomain = "";
        } else {
            // Buscar role atualizada
            auto roleOpt = tenantRepository_->getUserRoleInTenant(userId, tenantId);
            if (roleOpt.has_value()) {
                role = roleOpt.value();
            }
        }
    }
    
    // 8. Gerar novos tokens
    std::string newAccessToken;
    std::string newRefreshToken;
    
    if (!tenantId.empty()) {
        // Com tenant context
        newAccessToken = jwtService_->generateTokenForTenant(
            user.getId(),
            user.getEmail().value(),
            tenantId,
            tenantSubdomain,
            role
        );
    } else {
        // Sem tenant context
        newAccessToken = jwtService_->generateToken(
            user.getId(),
            user.getEmail().value(),
            role
        );
    }
    
    // Gerar novo refresh token
    newRefreshToken = jwtService_->generateRefreshToken(user.getId());
    
    LOG_INFO("Token refreshed for user: " + user.getId() + 
             (tenantId.empty() ? "" : " @ tenant " + tenantId));
    
    return RefreshTokenResult{
        newAccessToken,
        newRefreshToken
    };
}

} // namespace Domains::IAM::UseCases

