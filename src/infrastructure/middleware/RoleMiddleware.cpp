#include "RoleMiddleware.hpp"
#include "../../core/utils/LoggerNew.hpp"
#include "../../core/json/Json.hpp"

namespace Core::Http {

bool hasRole(const std::string& userRole, const std::vector<std::string>& allowedRoles) {
    return std::find(allowedRoles.begin(), allowedRoles.end(), userRole) != allowedRoles.end();
}

MiddlewareFunction createRoleMiddleware(const std::vector<std::string>& allowedRoles) {
    return [allowedRoles](Request& req, Response& res, NextFunction next) {
        // 1. Verificar se o AuthMiddleware já extraiu o user_role
        std::string userRole = req.getCustomData("user_role");
        
        if (userRole.empty()) {
            // Talvez o usuário não esteja autenticado ou AuthMiddleware não foi executado
            LOG_WARNING("RoleMiddleware: user_role not found in request. Did you forget AuthMiddleware?");
            
            auto json = Core::Json::makeObject();
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString("Authentication required");
            
            res = Response(StatusCode::Unauthorized).json(*json);
            return;
        }
        
        // 2. Verificar se usuário tem uma das roles permitidas
        if (!hasRole(userRole, allowedRoles)) {
            // Construir lista de roles para mensagem
            std::string rolesStr;
            for (size_t i = 0; i < allowedRoles.size(); ++i) {
                if (i > 0) rolesStr += ", ";
                rolesStr += allowedRoles[i];
            }
            
            LOG_WARNING("RoleMiddleware: Access denied. User role '" + userRole + 
                       "' not in allowed roles: [" + rolesStr + "]");
            
            auto json = Core::Json::makeObject();
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString("Insufficient permissions. Required role: " + rolesStr);
            
            res = Response(StatusCode::Forbidden).json(*json);
            return;
        }
        
        // 3. Role OK, continuar
        LOG_DEBUG("RoleMiddleware: Access granted for role '" + userRole + "'");
        next();
    };
}

} // namespace Core::Http

