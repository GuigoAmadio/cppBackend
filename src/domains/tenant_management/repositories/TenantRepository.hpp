#pragma once

#include "../../tenant_management/entities/Tenant.hpp"
#include "../../../core/database/ConnectionPool.hpp"
#include <memory>
#include <optional>
#include <vector>
#include <map>

namespace Domains::TenantManagement::Repositories {

/**
 * @brief Repository para gerenciar Tenants
 * 
 * Responsável por todas as operações de banco de dados relacionadas a tenants
 * e a relação user-tenant (user_tenants).
 */
class TenantRepository {
public:
    explicit TenantRepository(std::shared_ptr<Core::Database::ConnectionPool> pool);

    // === OPERAÇÕES BÁSICAS DE TENANT ===
    
    /**
     * @brief Busca tenant por subdomain
     * @param subdomain Subdomain do tenant (ex: "demo")
     * @return Tenant se encontrado, std::nullopt se não encontrado
     */
    std::optional<Entities::Tenant> findBySubdomain(const std::string& subdomain);

    /**
     * @brief Busca tenant por ID
     * @param id ID do tenant
     * @return Tenant se encontrado, std::nullopt se não encontrado
     */
    std::optional<Entities::Tenant> findById(const std::string& id);

    /**
     * @brief Cria um novo tenant
     * @param tenant Tenant a ser criado
     * @return ID do tenant criado
     */
    std::string create(const Entities::Tenant& tenant);
    
    /**
     * @brief Cria um novo tenant (versão simplificada)
     * @param id ID do tenant
     * @param name Nome do tenant
     * @param subdomain Subdomain único
     * @return true se criado com sucesso, false se falhar
     */
    bool createTenant(const std::string& id, const std::string& name, const std::string& subdomain);

    /**
     * @brief Atualiza um tenant existente
     * @param tenant Tenant com dados atualizados
     */
    void update(const Entities::Tenant& tenant);

    // === OPERAÇÕES DE USER-TENANT ===

    /**
     * @brief Verifica se um user tem acesso a um tenant
     * @param userId ID do user
     * @param tenantId ID do tenant
     * @return true se tem acesso ativo, false caso contrário
     */
    bool userHasAccessToTenant(const std::string& userId, const std::string& tenantId);

    /**
     * @brief Obtém a role de um user em um tenant específico
     * @param userId ID do user
     * @param tenantId ID do tenant
     * @return Role como string ("owner", "admin", "user", "viewer") ou std::nullopt
     */
    std::optional<std::string> getUserRoleInTenant(
        const std::string& userId,
        const std::string& tenantId
    );

    /**
     * @brief Adiciona um user a um tenant com uma role
     * @param userId ID do user
     * @param tenantId ID do tenant
     * @param role Role do user no tenant
     * @param invitedBy ID do user que fez o convite (opcional)
     */
    void addUserToTenant(
        const std::string& userId,
        const std::string& tenantId,
        const std::string& role,
        const std::optional<std::string>& invitedBy = std::nullopt
    );

    /**
     * @brief Remove um user de um tenant
     * @param userId ID do user
     * @param tenantId ID do tenant
     */
    void removeUserFromTenant(const std::string& userId, const std::string& tenantId);

    /**
     * @brief Atualiza a role de um user em um tenant
     * @param userId ID do user
     * @param tenantId ID do tenant
     * @param newRole Nova role
     */
    void updateUserRole(
        const std::string& userId,
        const std::string& tenantId,
        const std::string& newRole
    );

    /**
     * @brief Lista todos os tenants de um user
     * @param userId ID do user
     * @return Vector de tenants com informações de role
     */
    std::vector<std::map<std::string, std::string>> getUserTenants(const std::string& userId);

    /**
     * @brief Lista todos os users de um tenant
     * @param tenantId ID do tenant
     * @return Vector com informações de users e suas roles
     */
    std::vector<std::map<std::string, std::string>> getTenantUsers(const std::string& tenantId);
    
    /**
     * @brief Lista todos os membros de um tenant (versão estruturada)
     * @param tenantId ID do tenant
     * @return Vector com informações estruturadas dos membros
     */
    std::vector<std::map<std::string, std::string>> getTenantMembers(const std::string& tenantId);

    /**
     * @brief Conta quantos users ativos existem em um tenant
     * @param tenantId ID do tenant
     * @return Número de users ativos
     */
    int countActiveUsers(const std::string& tenantId);

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;

    // Helper para converter resultado SQL em Tenant
    Entities::Tenant mapToTenant(const Core::Database::QueryResult& result, int row);
};

} // namespace Domains::TenantManagement::Repositories

