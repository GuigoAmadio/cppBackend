#include "CustomerController.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>

using namespace Core::Http;

namespace Domain {
namespace Customer {

Response CustomerController::createCustomer(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[CustomerController] createCustomer called");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            LOG_ERROR("[CustomerController] Invalid JSON body");
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto& bodyObj = bodyJson->asObject();
        LOG_DEBUG("[CustomerController] JSON parsed successfully");
        
        // Extrair dados do JWT (AuthMiddleware)
        std::string userId = req.getCustomData("user_id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        if (tenantId.empty()) {
            LOG_ERROR("[CustomerController] Tenant ID not found in request");
            json->asObject()["error"] = Core::Json::makeString("Tenant ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        // Criar DTO
        CreateCustomerDto dto;
        dto.tenantId = tenantId;
        dto.createdBy = userId;
        
        // Campos obrigatórios
        if (bodyObj.find("name") != bodyObj.end() && bodyObj["name"]->isString()) {
            dto.name = bodyObj["name"]->asString();
        }
        if (bodyObj.find("type") != bodyObj.end() && bodyObj["type"]->isString()) {
            dto.type = bodyObj["type"]->asString();
        }
        
        // Campos opcionais
        if (bodyObj.find("workspace_id") != bodyObj.end() && bodyObj["workspace_id"]->isString()) {
            dto.workspaceId = bodyObj["workspace_id"]->asString();
        }
        if (bodyObj.find("email") != bodyObj.end() && bodyObj["email"]->isString()) {
            dto.email = bodyObj["email"]->asString();
        }
        if (bodyObj.find("phone") != bodyObj.end() && bodyObj["phone"]->isString()) {
            dto.phone = bodyObj["phone"]->asString();
        }
        if (bodyObj.find("document") != bodyObj.end() && bodyObj["document"]->isString()) {
            dto.document = bodyObj["document"]->asString();
        }
        if (bodyObj.find("address") != bodyObj.end() && bodyObj["address"]->isString()) {
            dto.address = bodyObj["address"]->asString();
        }
        if (bodyObj.find("notes") != bodyObj.end() && bodyObj["notes"]->isString()) {
            dto.notes = bodyObj["notes"]->asString();
        }
        
        LOG_DEBUG("[CustomerController] Calling CreateCustomerUseCase");
        CreateCustomerUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success) {
            LOG_ERROR("[CustomerController] Failed to create customer: " + result.message);
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        LOG_DEBUG("[CustomerController] Customer created: " + result.customerId);
        json->asObject()["id"] = Core::Json::makeString(result.customerId);
        json->asObject()["message"] = Core::Json::makeString(result.message);
        return Response(StatusCode::Created).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[CustomerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CustomerController::getCustomer(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[CustomerController] getCustomer called");
        
        std::string customerId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        LOG_DEBUG("[CustomerController] customer_id: " + customerId);
        
        GetCustomerDto dto;
        dto.customerId = customerId;
        dto.tenantId = tenantId;
        
        GetCustomerUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success || !result.customer.has_value()) {
            LOG_WARNING("[CustomerController] Customer not found");
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::NotFound).json(*json);
        }
        
        auto& customer = result.customer.value();
        
        // Montar resposta JSON
        json->asObject()["id"] = Core::Json::makeString(customer.getId());
        json->asObject()["tenant_id"] = Core::Json::makeString(customer.getTenantId());
        json->asObject()["name"] = Core::Json::makeString(customer.getName());
        json->asObject()["type"] = Core::Json::makeString(customer.getType().toString());
        json->asObject()["status"] = Core::Json::makeString(customer.getStatus().toString());
        
        if (customer.getWorkspaceId().has_value()) {
            json->asObject()["workspace_id"] = Core::Json::makeString(*customer.getWorkspaceId());
        }
        if (customer.getEmail().has_value()) {
            json->asObject()["email"] = Core::Json::makeString(*customer.getEmail());
        }
        if (customer.getPhone().has_value()) {
            json->asObject()["phone"] = Core::Json::makeString(*customer.getPhone());
        }
        if (customer.getDocument().has_value()) {
            json->asObject()["document"] = Core::Json::makeString(*customer.getDocument());
        }
        if (customer.getAddress().has_value()) {
            json->asObject()["address"] = Core::Json::makeString(*customer.getAddress());
        }
        if (customer.getNotes().has_value()) {
            json->asObject()["notes"] = Core::Json::makeString(*customer.getNotes());
        }
        if (customer.getCreatedAt().has_value()) {
            json->asObject()["created_at"] = Core::Json::makeString(*customer.getCreatedAt());
        }
        if (customer.getUpdatedAt().has_value()) {
            json->asObject()["updated_at"] = Core::Json::makeString(*customer.getUpdatedAt());
        }
        
        LOG_DEBUG("[CustomerController] Customer retrieved successfully");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[CustomerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CustomerController::listCustomers(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[CustomerController] listCustomers called");
        
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string limitStr = getQueryParam(req, "limit");
        std::string offsetStr = getQueryParam(req, "offset");
        
        ListCustomersDto dto;
        dto.tenantId = tenantId;
        dto.limit = limitStr.empty() ? 100 : std::stoi(limitStr);
        dto.offset = offsetStr.empty() ? 0 : std::stoi(offsetStr);
        
        ListCustomersUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success) {
            LOG_ERROR("[CustomerController] Failed: " + result.message);
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto customersArray = Core::Json::makeArray();
        for (const auto& customer : result.customers) {
            auto custObj = Core::Json::makeObject();
            custObj->asObject()["id"] = Core::Json::makeString(customer.getId());
            custObj->asObject()["name"] = Core::Json::makeString(customer.getName());
            custObj->asObject()["type"] = Core::Json::makeString(customer.getType().toString());
            custObj->asObject()["status"] = Core::Json::makeString(customer.getStatus().toString());
            
            if (customer.getEmail().has_value()) {
                custObj->asObject()["email"] = Core::Json::makeString(*customer.getEmail());
            }
            if (customer.getPhone().has_value()) {
                custObj->asObject()["phone"] = Core::Json::makeString(*customer.getPhone());
            }
            
            customersArray->asArray().push_back(std::move(custObj));
        }
        
        json->asObject()["customers"] = std::move(customersArray);
        json->asObject()["total"] = Core::Json::makeNumber(result.total);
        
        LOG_DEBUG("[CustomerController] Listed " + std::to_string(result.total) + " customers");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[CustomerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CustomerController::searchCustomers(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[CustomerController] searchCustomers called");
        
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string searchQuery = getQueryParam(req, "q");
        std::string limitStr = getQueryParam(req, "limit");
        
        if (searchQuery.empty()) {
            LOG_WARNING("[CustomerController] Search query is empty");
            json->asObject()["error"] = Core::Json::makeString("Query parameter 'q' is required");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        LOG_DEBUG("[CustomerController] Search query: " + searchQuery);
        
        ListCustomersDto dto;
        dto.tenantId = tenantId;
        dto.searchQuery = searchQuery;
        dto.limit = limitStr.empty() ? 50 : std::stoi(limitStr);
        
        ListCustomersUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        auto customersArray = Core::Json::makeArray();
        for (const auto& customer : result.customers) {
            auto custObj = Core::Json::makeObject();
            custObj->asObject()["id"] = Core::Json::makeString(customer.getId());
            custObj->asObject()["name"] = Core::Json::makeString(customer.getName());
            custObj->asObject()["type"] = Core::Json::makeString(customer.getType().toString());
            
            if (customer.getEmail().has_value()) {
                custObj->asObject()["email"] = Core::Json::makeString(*customer.getEmail());
            }
            if (customer.getDocument().has_value()) {
                custObj->asObject()["document"] = Core::Json::makeString(*customer.getDocument());
            }
            
            customersArray->asArray().push_back(std::move(custObj));
        }
        
        json->asObject()["customers"] = std::move(customersArray);
        json->asObject()["total"] = Core::Json::makeNumber(result.total);
        
        LOG_DEBUG("[CustomerController] Search found " + std::to_string(result.total) + " customers");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[CustomerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CustomerController::updateCustomer(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[CustomerController] updateCustomer called");
        
        std::string customerId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto& bodyObj = bodyJson->asObject();
        
        UpdateCustomerDto dto;
        dto.customerId = customerId;
        dto.tenantId = tenantId;
        
        // Campos opcionais
        if (bodyObj.find("name") != bodyObj.end() && bodyObj["name"]->isString()) {
            dto.name = bodyObj["name"]->asString();
        }
        if (bodyObj.find("email") != bodyObj.end() && bodyObj["email"]->isString()) {
            dto.email = bodyObj["email"]->asString();
        }
        if (bodyObj.find("phone") != bodyObj.end() && bodyObj["phone"]->isString()) {
            dto.phone = bodyObj["phone"]->asString();
        }
        if (bodyObj.find("document") != bodyObj.end() && bodyObj["document"]->isString()) {
            dto.document = bodyObj["document"]->asString();
        }
        if (bodyObj.find("status") != bodyObj.end() && bodyObj["status"]->isString()) {
            dto.status = bodyObj["status"]->asString();
        }
        if (bodyObj.find("address") != bodyObj.end() && bodyObj["address"]->isString()) {
            dto.address = bodyObj["address"]->asString();
        }
        if (bodyObj.find("notes") != bodyObj.end() && bodyObj["notes"]->isString()) {
            dto.notes = bodyObj["notes"]->asString();
        }
        
        UpdateCustomerUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success) {
            LOG_ERROR("[CustomerController] Update failed: " + result.message);
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        LOG_DEBUG("[CustomerController] Customer updated");
        json->asObject()["message"] = Core::Json::makeString(result.message);
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[CustomerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CustomerController::deleteCustomer(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[CustomerController] deleteCustomer called");
        
        std::string customerId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        DeleteCustomerDto dto;
        dto.customerId = customerId;
        dto.tenantId = tenantId;
        
        DeleteCustomerUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success) {
            LOG_ERROR("[CustomerController] Delete failed: " + result.message);
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        LOG_DEBUG("[CustomerController] Customer deleted (soft)");
        json->asObject()["message"] = Core::Json::makeString(result.message);
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[CustomerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

} // namespace Customer
} // namespace Domain

