#include "TenantMiddleware.hpp"
#include "Middleware.hpp"
#include "../utils/LoggerNew.hpp"
#include <algorithm>

namespace Core::Http {

std::string extractSubdomain(const std::string& host) {
    // Remover porta se presente (ex: "demo.localhost:8080" -> "demo.localhost")
    std::string hostWithoutPort = host;
    size_t colonPos = host.find(':');
    if (colonPos != std::string::npos) {
        hostWithoutPort = host.substr(0, colonPos);
    }
    
    LOG_DEBUG("extractSubdomain: Host without port = " + hostWithoutPort);
    
    // Casos especiais:
    // - "localhost" -> sem subdomain
    // - "127.0.0.1" -> sem subdomain
    // - "demo.localhost" -> subdomain = "demo"
    // - "app.example.com" -> subdomain = "app"
    
    if (hostWithoutPort == "localhost" || hostWithoutPort == "127.0.0.1") {
        return "";
    }
    
    // Dividir por "."
    size_t firstDot = hostWithoutPort.find('.');
    if (firstDot == std::string::npos) {
        // Sem ponto -> sem subdomain
        return "";
    }
    
    // Extrair primeira parte antes do ponto
    std::string subdomain = hostWithoutPort.substr(0, firstDot);
    
    // Validar se subdomain não está vazio e é válido
    if (subdomain.empty()) {
        return "";
    }
    
    // Converter para lowercase
    std::transform(subdomain.begin(), subdomain.end(), subdomain.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    return subdomain;
}

MiddlewareFunction createTenantMiddleware(
    std::shared_ptr<Domains::Identity::Repositories::TenantRepository> tenantRepo
) {
    return [tenantRepo](Request& req, Response& res, NextFunction next) {
        // 1. Extrair header Host
        std::string host = req.getHeader("Host");
        
        if (host.empty()) {
            host = req.getHeader("host"); // Try lowercase
        }
        
        if (host.empty()) {
            LOG_WARNING("TenantMiddleware: No Host header found");
            // Continue sem tenant
            next();
            return;
        }
        
        LOG_DEBUG("TenantMiddleware: Host header = " + host);
        
        // 2. Extrair subdomain
        std::string subdomain = extractSubdomain(host);
        
        if (subdomain.empty()) {
            LOG_DEBUG("TenantMiddleware: No subdomain found in Host header");
            // Continue sem tenant
            next();
            return;
        }
        
        LOG_INFO("TenantMiddleware: Subdomain extracted = " + subdomain);
        
        // 3. Buscar tenant no banco
        try {
            auto tenantOpt = tenantRepo->findBySubdomain(subdomain);
            
            if (!tenantOpt.has_value()) {
                LOG_WARNING("TenantMiddleware: Tenant not found for subdomain: " + subdomain);
                
                // Retornar erro 404 - Tenant não encontrado
                auto json = Core::Json::makeObject();
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString("Tenant not found: " + subdomain);
                
                res = Response(StatusCode::NotFound).json(*json);
                return; // Não chamar next()
            }
            
            auto tenant = tenantOpt.value();
            
            // 4. Verificar se tenant está ativo
            if (!tenant.isActive()) {
                LOG_WARNING("TenantMiddleware: Tenant is inactive: " + subdomain);
                
                auto json = Core::Json::makeObject();
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString("Tenant is inactive");
                
                res = Response(StatusCode::Forbidden).json(*json);
                return; // Não chamar next()
            }
            
            LOG_INFO("TenantMiddleware: Tenant resolved - ID: " + tenant.getId() + ", Name: " + tenant.getName());
            
            // 5. Adicionar informações do tenant ao request
            req.setCustomData("tenant_id", tenant.getId());
            req.setCustomData("tenant_subdomain", tenant.getSubdomain());
            req.setCustomData("tenant_name", tenant.getName());
            req.setCustomData("tenant_plan", Domains::Identity::Entities::Tenant::planToString(tenant.getPlan()));
            
            // 6. Continuar para próximo middleware/handler
            next();
            
        } catch (const std::exception& e) {
            LOG_ERROR("TenantMiddleware: Error resolving tenant: " + std::string(e.what()));
            
            auto json = Core::Json::makeObject();
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString("Internal error resolving tenant");
            
            res = Response(StatusCode::InternalServerError).json(*json);
            return; // Não chamar next()
        }
    };
}

} // namespace Core::Http
