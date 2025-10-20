#include "TenantRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>

namespace Domains::Identity::Repositories {

TenantRepository::TenantRepository(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {}

std::optional<Entities::Tenant> TenantRepository::findBySubdomain(const std::string& subdomain) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT id, name, subdomain, plan, max_users, is_active, settings, created_at, updated_at
        FROM tenants
        WHERE subdomain = $1 AND is_active = true
    )";
    
    auto result = conn->executeParams(sql, {subdomain});
    
    if (result.isSuccess() && result.rowCount() > 0) {
        return mapToTenant(result, 0);
    }
    
    return std::nullopt;
}

std::optional<Entities::Tenant> TenantRepository::findById(const std::string& id) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT id, name, subdomain, plan, max_users, is_active, settings, created_at, updated_at
        FROM tenants
        WHERE id = $1
    )";
    
    auto result = conn->executeParams(sql, {id});
    
    if (result.isSuccess() && result.rowCount() > 0) {
        return mapToTenant(result, 0);
    }
    
    return std::nullopt;
}

std::string TenantRepository::create(const Entities::Tenant& tenant) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        INSERT INTO tenants (id, name, subdomain, plan, max_users, is_active, settings, created_at, updated_at)
        VALUES ($1, $2, $3, $4, $5, $6, $7, NOW(), NOW())
        RETURNING id
    )";
    
    auto result = conn->executeParams(sql, {
        tenant.getId(),
        tenant.getName(),
        tenant.getSubdomain(),
        Entities::Tenant::planToString(tenant.getPlan()),
        std::to_string(tenant.getMaxUsers()),
        tenant.isActive() ? "true" : "false",
        tenant.getSettings()
    });
    
    if (result.isSuccess() && result.rowCount() > 0) {
        LOG_INFO("Tenant created: " + tenant.getName() + " (" + tenant.getSubdomain() + ")");
        return result.getValue(0, 0);
    }
    
    throw std::runtime_error("Failed to create tenant");
}

void TenantRepository::update(const Entities::Tenant& tenant) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        UPDATE tenants
        SET name = $1, subdomain = $2, plan = $3, max_users = $4, is_active = $5, settings = $6, updated_at = NOW()
        WHERE id = $7
    )";
    
    auto result = conn->executeParams(sql, {
        tenant.getName(),
        tenant.getSubdomain(),
        Entities::Tenant::planToString(tenant.getPlan()),
        std::to_string(tenant.getMaxUsers()),
        tenant.isActive() ? "true" : "false",
        tenant.getSettings(),
        tenant.getId()
    });
    
    if (!result.isSuccess()) {
        throw std::runtime_error("Failed to update tenant");
    }
    
    LOG_INFO("Tenant updated: " + tenant.getName());
}

bool TenantRepository::userHasAccessToTenant(const std::string& userId, const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT 1 FROM user_tenants
        WHERE user_id = $1 AND tenant_id = $2 AND is_active = true
    )";
    
    auto result = conn->executeParams(sql, {userId, tenantId});
    
    return result.isSuccess() && result.rowCount() > 0;
}

std::optional<std::string> TenantRepository::getUserRoleInTenant(const std::string& userId, const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT role FROM user_tenants
        WHERE user_id = $1 AND tenant_id = $2 AND is_active = true
    )";
    
    auto result = conn->executeParams(sql, {userId, tenantId});
    
    if (result.isSuccess() && result.rowCount() > 0) {
        return result.getValue(0, 0);
    }
    
    return std::nullopt;
}

void TenantRepository::addUserToTenant(
    const std::string& userId, 
    const std::string& tenantId, 
    const std::string& role,
    const std::optional<std::string>& addedBy
) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        INSERT INTO user_tenants (user_id, tenant_id, role, is_active, created_at, updated_at)
        VALUES ($1, $2, $3, true, NOW(), NOW())
        ON CONFLICT (user_id, tenant_id) 
        DO UPDATE SET role = EXCLUDED.role, is_active = true, updated_at = NOW()
    )";
    
    auto result = conn->executeParams(sql, {userId, tenantId, role});
    
    if (!result.isSuccess()) {
        throw std::runtime_error("Failed to add user to tenant");
    }
    
    LOG_INFO("User " + userId + " added to tenant " + tenantId + " with role " + role);
}

void TenantRepository::removeUserFromTenant(const std::string& userId, const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        DELETE FROM user_tenants
        WHERE user_id = $1 AND tenant_id = $2
    )";
    
    auto result = conn->executeParams(sql, {userId, tenantId});
    
    if (!result.isSuccess()) {
        throw std::runtime_error("Failed to remove user from tenant");
    }
    
    LOG_INFO("User " + userId + " removed from tenant " + tenantId);
}

void TenantRepository::updateUserRole(const std::string& userId, const std::string& tenantId, const std::string& newRole) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        UPDATE user_tenants
        SET role = $1, updated_at = NOW()
        WHERE user_id = $2 AND tenant_id = $3
    )";
    
    auto result = conn->executeParams(sql, {newRole, userId, tenantId});
    
    if (!result.isSuccess()) {
        throw std::runtime_error("Failed to update user role");
    }
    
    LOG_INFO("User " + userId + " role updated to " + newRole + " in tenant " + tenantId);
}

std::vector<std::map<std::string, std::string>> TenantRepository::getUserTenants(const std::string& userId) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT t.id, t.name, t.subdomain, t.plan, ut.role
        FROM tenants t
        INNER JOIN user_tenants ut ON t.id = ut.tenant_id
        WHERE ut.user_id = $1 AND ut.is_active = true AND t.is_active = true
        ORDER BY t.name
    )";
    
    auto result = conn->executeParams(sql, {userId});
    
    std::vector<std::map<std::string, std::string>> tenants;
    
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); i++) {
            std::map<std::string, std::string> tenant;
            tenant["id"] = result.getValue(i, 0);
            tenant["name"] = result.getValue(i, 1);
            tenant["subdomain"] = result.getValue(i, 2);
            tenant["plan"] = result.getValue(i, 3);
            tenant["role"] = result.getValue(i, 4);
            tenants.push_back(tenant);
        }
    }
    
    return tenants;
}

std::vector<std::map<std::string, std::string>> TenantRepository::getTenantUsers(const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT u.id, u.email, u.name, ut.role, ut.is_active
        FROM users u
        INNER JOIN user_tenants ut ON u.id = ut.user_id
        WHERE ut.tenant_id = $1
        ORDER BY u.name
    )";
    
    auto result = conn->executeParams(sql, {tenantId});
    
    std::vector<std::map<std::string, std::string>> users;
    
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); i++) {
            std::map<std::string, std::string> user;
            user["id"] = result.getValue(i, 0);
            user["email"] = result.getValue(i, 1);
            user["name"] = result.getValue(i, 2);
            user["role"] = result.getValue(i, 3);
            user["is_active"] = result.getValue(i, 4);
            users.push_back(user);
        }
    }
    
    return users;
}

std::vector<std::map<std::string, std::string>> TenantRepository::getTenantMembers(const std::string& tenantId) {
    // Alias para getTenantUsers (mantém compatibilidade)
    return getTenantUsers(tenantId);
}

int TenantRepository::countActiveUsers(const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT COUNT(*) FROM user_tenants
        WHERE tenant_id = $1 AND is_active = true
    )";
    
    auto result = conn->executeParams(sql, {tenantId});
    
    if (result.isSuccess() && result.rowCount() > 0) {
        return std::stoi(result.getValue(0, 0));
    }
    
    return 0;
}

Entities::Tenant TenantRepository::mapToTenant(const Core::Database::QueryResult& result, int row) {
    std::string id = result.getValue(row, 0);
    std::string name = result.getValue(row, 1);
    std::string subdomain = result.getValue(row, 2);
    std::string planStr = result.getValue(row, 3);
    int maxUsers = std::stoi(result.getValue(row, 4));
    std::string isActiveStr = result.getValue(row, 5);
    std::string settings = result.getValue(row, 6);
    std::string createdAtStr = result.getValue(row, 7);
    std::string updatedAtStr = result.getValue(row, 8);
    
    Entities::Tenant::Plan plan = Entities::Tenant::stringToPlan(planStr);
    bool isActive = (isActiveStr == "t" || isActiveStr == "true" || isActiveStr == "1");
    
    // Parse timestamps (simplified - just use NOW for now)
    auto createdAt = std::chrono::system_clock::now();
    auto updatedAt = std::chrono::system_clock::now();
    
    return Entities::Tenant(id, name, subdomain, plan, maxUsers, isActive, settings, createdAt, updatedAt);
}

} // namespace Domains::Identity::Repositories
