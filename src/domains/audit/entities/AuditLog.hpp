#pragma once

#include <string>
#include <chrono>
#include <optional>

namespace Domains::Audit::Entities {

/**
 * @brief Entity para Audit Log
 * 
 * Representa um registro de auditoria no sistema.
 */
class AuditLog {
public:
    AuditLog(
        const std::string& id,
        const std::optional<std::string>& userId,
        const std::optional<std::string>& tenantId,
        const std::string& action,
        const std::string& resource,
        const std::optional<std::string>& resourceId,
        const std::string& details, // JSON string
        const std::string& ipAddress,
        const std::string& userAgent,
        const std::chrono::system_clock::time_point& createdAt
    );
    
    // Getters
    std::string getId() const { return id_; }
    std::optional<std::string> getUserId() const { return userId_; }
    std::optional<std::string> getTenantId() const { return tenantId_; }
    std::string getAction() const { return action_; }
    std::string getResource() const { return resource_; }
    std::optional<std::string> getResourceId() const { return resourceId_; }
    std::string getDetails() const { return details_; }
    std::string getIpAddress() const { return ipAddress_; }
    std::string getUserAgent() const { return userAgent_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt_; }

private:
    std::string id_;
    std::optional<std::string> userId_;
    std::optional<std::string> tenantId_;
    std::string action_;
    std::string resource_;
    std::optional<std::string> resourceId_;
    std::string details_; // JSON string
    std::string ipAddress_;
    std::string userAgent_;
    std::chrono::system_clock::time_point createdAt_;
};

} // namespace Domains::Audit::Entities

