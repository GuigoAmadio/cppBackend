#include "AuditLogRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <iomanip>
#include <sstream>

namespace Domains::Identity::Repositories {

std::chrono::system_clock::time_point parseTimestampAudit(const std::string& str) {
    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

AuditLogRepository::AuditLogRepository(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {}

void AuditLogRepository::log(
    const std::optional<std::string>& userId,
    const std::optional<std::string>& tenantId,
    const std::string& action,
    const std::string& resource,
    const std::optional<std::string>& resourceId,
    const std::string& details,
    const std::string& ipAddress,
    const std::string& userAgent
) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        INSERT INTO audit_logs (user_id, tenant_id, action, resource, resource_id, details, ip_address, user_agent)
        VALUES ($1, $2, $3, $4, $5, $6::jsonb, $7, $8)
    )";
    
    std::vector<std::string> params = {
        userId.value_or(""),
        tenantId.value_or(""),
        action,
        resource,
        resourceId.value_or(""),
        details.empty() ? "{}" : details,
        ipAddress,
        userAgent
    };
    
    try {
        conn->executeParams(sql, params);
        LOG_DEBUG("Audit log created: " + action + " on " + resource);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create audit log: " + std::string(e.what()));
    }
}

std::vector<Entities::AuditLog> AuditLogRepository::findByUser(
    const std::string& userId,
    int limit,
    int offset
) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT id, user_id, tenant_id, action, resource, resource_id, 
               details::text, ip_address, user_agent, created_at
        FROM audit_logs
        WHERE user_id = $1
        ORDER BY created_at DESC
        LIMIT $2 OFFSET $3
    )";
    
    std::vector<std::string> params = {userId, std::to_string(limit), std::to_string(offset)};
    auto result = conn->executeParams(sql, params);
    
    std::vector<Entities::AuditLog> logs;
    
    if (!result.isSuccess()) {
        return logs;
    }
    
    for (int i = 0; i < result.rowCount(); i++) {
        std::optional<std::string> userIdOpt = result.isNull(i, 1) ? std::nullopt : std::optional<std::string>(result.getValue(i, 1));
        std::optional<std::string> tenantIdOpt = result.isNull(i, 2) ? std::nullopt : std::optional<std::string>(result.getValue(i, 2));
        std::optional<std::string> resourceIdOpt = result.isNull(i, 5) ? std::nullopt : std::optional<std::string>(result.getValue(i, 5));
        
        logs.emplace_back(
            result.getValue(i, 0),  // id
            userIdOpt,
            tenantIdOpt,
            result.getValue(i, 3),  // action
            result.getValue(i, 4),  // resource
            resourceIdOpt,
            result.getValue(i, 6),  // details
            result.getValue(i, 7),  // ip_address
            result.getValue(i, 8),  // user_agent
            parseTimestampAudit(result.getValue(i, 9))  // created_at
        );
    }
    
    return logs;
}

std::vector<Entities::AuditLog> AuditLogRepository::findByTenant(
    const std::string& tenantId,
    int limit,
    int offset
) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT id, user_id, tenant_id, action, resource, resource_id, 
               details::text, ip_address, user_agent, created_at
        FROM audit_logs
        WHERE tenant_id = $1
        ORDER BY created_at DESC
        LIMIT $2 OFFSET $3
    )";
    
    std::vector<std::string> params = {tenantId, std::to_string(limit), std::to_string(offset)};
    auto result = conn->executeParams(sql, params);
    
    std::vector<Entities::AuditLog> logs;
    
    if (!result.isSuccess()) {
        return logs;
    }
    
    for (int i = 0; i < result.rowCount(); i++) {
        std::optional<std::string> userIdOpt = result.isNull(i, 1) ? std::nullopt : std::optional<std::string>(result.getValue(i, 1));
        std::optional<std::string> tenantIdOpt = result.isNull(i, 2) ? std::nullopt : std::optional<std::string>(result.getValue(i, 2));
        std::optional<std::string> resourceIdOpt = result.isNull(i, 5) ? std::nullopt : std::optional<std::string>(result.getValue(i, 5));
        
        logs.emplace_back(
            result.getValue(i, 0),
            userIdOpt,
            tenantIdOpt,
            result.getValue(i, 3),
            result.getValue(i, 4),
            resourceIdOpt,
            result.getValue(i, 6),
            result.getValue(i, 7),
            result.getValue(i, 8),
            parseTimestampAudit(result.getValue(i, 9))
        );
    }
    
    return logs;
}

std::vector<Entities::AuditLog> AuditLogRepository::findByAction(
    const std::string& action,
    int limit,
    int offset
) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT id, user_id, tenant_id, action, resource, resource_id, 
               details::text, ip_address, user_agent, created_at
        FROM audit_logs
        WHERE action = $1
        ORDER BY created_at DESC
        LIMIT $2 OFFSET $3
    )";
    
    std::vector<std::string> params = {action, std::to_string(limit), std::to_string(offset)};
    auto result = conn->executeParams(sql, params);
    
    std::vector<Entities::AuditLog> logs;
    
    if (!result.isSuccess()) {
        return logs;
    }
    
    for (int i = 0; i < result.rowCount(); i++) {
        std::optional<std::string> userIdOpt = result.isNull(i, 1) ? std::nullopt : std::optional<std::string>(result.getValue(i, 1));
        std::optional<std::string> tenantIdOpt = result.isNull(i, 2) ? std::nullopt : std::optional<std::string>(result.getValue(i, 2));
        std::optional<std::string> resourceIdOpt = result.isNull(i, 5) ? std::nullopt : std::optional<std::string>(result.getValue(i, 5));
        
        logs.emplace_back(
            result.getValue(i, 0),
            userIdOpt,
            tenantIdOpt,
            result.getValue(i, 3),
            result.getValue(i, 4),
            resourceIdOpt,
            result.getValue(i, 6),
            result.getValue(i, 7),
            result.getValue(i, 8),
            parseTimestampAudit(result.getValue(i, 9))
        );
    }
    
    return logs;
}

} // namespace Domains::Identity::Repositories
