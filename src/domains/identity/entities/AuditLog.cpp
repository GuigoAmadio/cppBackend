#include "AuditLog.hpp"

namespace Domains::Identity::Entities {

AuditLog::AuditLog(
    const std::string& id,
    const std::optional<std::string>& userId,
    const std::optional<std::string>& tenantId,
    const std::string& action,
    const std::string& resource,
    const std::optional<std::string>& resourceId,
    const std::string& details,
    const std::string& ipAddress,
    const std::string& userAgent,
    const std::chrono::system_clock::time_point& createdAt
) : id_(id),
    userId_(userId),
    tenantId_(tenantId),
    action_(action),
    resource_(resource),
    resourceId_(resourceId),
    details_(details),
    ipAddress_(ipAddress),
    userAgent_(userAgent),
    createdAt_(createdAt) {}

} // namespace Domains::Identity::Entities

