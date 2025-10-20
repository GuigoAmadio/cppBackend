#pragma once

#include "../services/JwtService.hpp"
#include "../repositories/UserRepository.hpp"
#include "../repositories/TenantRepository.hpp"
#include <memory>
#include <string>

namespace Domains::Identity::UseCases {

using namespace Services;
using namespace Repositories;

/**
 * @brief DTO para refresh de token
 */
struct RefreshTokenDto {
    std::string refreshToken;
};

/**
 * @brief Resultado do refresh
 */
struct RefreshTokenResult {
    std::string accessToken;
    std::string refreshToken;
};

/**
 * @brief Use Case: Renovar access token usando refresh token
 * 
 * Regras de negócio:
 * - Refresh token deve ser válido e não expirado
 * - Usuário deve existir e estar ativo
 * - Gera novo access token e novo refresh token
 * - Mantém contexto de tenant se presente
 */
class RefreshTokenUseCase {
public:
    explicit RefreshTokenUseCase(
        std::shared_ptr<JwtService> jwtService,
        std::shared_ptr<UserRepository> userRepository,
        std::shared_ptr<TenantRepository> tenantRepository
    );
    
    /**
     * @brief Executa o caso de uso
     * @throws std::runtime_error se refresh token inválido ou usuário não encontrado
     */
    RefreshTokenResult execute(const RefreshTokenDto& dto);

private:
    std::shared_ptr<JwtService> jwtService_;
    std::shared_ptr<UserRepository> userRepository_;
    std::shared_ptr<TenantRepository> tenantRepository_;
};

} // namespace Domains::Identity::UseCases

