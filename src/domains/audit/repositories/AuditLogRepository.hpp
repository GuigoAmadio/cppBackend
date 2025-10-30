#pragma once

#include "../../audit/entities/AuditLog.hpp"
#include "../../../core/database/ConnectionPool.hpp"
#include <string>
#include <memory>
#include <vector>
#include <optional>

namespace Domains::Audit::Repositories {

class AuditLogRepository {
public:
    explicit AuditLogRepository(std::shared_ptr<Core::Database::ConnectionPool> pool);
    
    /**
     * @brief Registra uma ação de auditoria
     * @param userId ID do usuário (opcional)
     * @param tenantId ID do tenant (opcional)
     * @param action Ação executada (ex: USER_LOGIN, USER_DELETED)
     * @param resource Tipo de recurso (ex: user, tenant, role)
     * @param resourceId ID do recurso afetado (opcional)
     * @param details Detalhes adicionais em JSON string
     * @param ipAddress IP do usuário
     * @param userAgent User-Agent do usuário
     */
    void log(
        const std::optional<std::string>& userId,
        const std::optional<std::string>& tenantId,
        const std::string& action,
        const std::string& resource,
        const std::optional<std::string>& resourceId,
        const std::string& details,
        const std::string& ipAddress,
        const std::string& userAgent
    );
    
    /**
     * @brief Busca logs por usuário
     * @param userId ID do usuário
     * @param limit Limite de resultados
     * @param offset Offset para paginação
     * @return Vector de AuditLogs
     */
    std::vector<Entities::AuditLog> findByUser(
        const std::string& userId,
        int limit = 50,
        int offset = 0
    );
    
    /**
     * @brief Busca logs por tenant
     * @param tenantId ID do tenant
     * @param limit Limite de resultados
     * @param offset Offset para paginação
     * @return Vector de AuditLogs
     */
    std::vector<Entities::AuditLog> findByTenant(
        const std::string& tenantId,
        int limit = 50,
        int offset = 0
    );
    
    /**
     * @brief Busca logs por ação
     * @param action Ação a buscar
     * @param limit Limite de resultados
     * @param offset Offset para paginação
     * @return Vector de AuditLogs
     */
    std::vector<Entities::AuditLog> findByAction(
        const std::string& action,
        int limit = 50,
        int offset = 0
    );

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
};

} // namespace Domains::Audit::Repositories

