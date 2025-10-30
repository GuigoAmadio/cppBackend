#include "InventoryController.hpp"
#include "../use_cases/RecordTransactionUseCase.hpp"
#include "../use_cases/GetInventoryHistoryUseCase.hpp"
#include "../use_cases/GetCurrentStockUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Inventory {

std::shared_ptr<Core::Json::JsonValue> InventoryController::transactionToJson(
    const InventoryTransaction& transaction
) {
    auto json = Core::Json::makeObject();
    auto& obj = json->asObject();
    
    obj["id"] = Core::Json::makeString(transaction.getId());
    obj["tenant_id"] = Core::Json::makeString(transaction.getTenantId());
    obj["product_id"] = Core::Json::makeString(transaction.getProductId());
    obj["type"] = Core::Json::makeString(transaction.getType().toString());
    obj["quantity"] = Core::Json::makeNumber(transaction.getQuantity());
    obj["quantity_before"] = Core::Json::makeNumber(transaction.getQuantityBefore());
    obj["quantity_after"] = Core::Json::makeNumber(transaction.getQuantityAfter());
    obj["created_by"] = Core::Json::makeString(transaction.getCreatedBy());
    
    if (transaction.getLocationId().has_value()) {
        obj["location_id"] = Core::Json::makeString(*transaction.getLocationId());
    }
    if (transaction.getReferenceType().has_value()) {
        obj["reference_type"] = Core::Json::makeString(transaction.getReferenceType()->toString());
    }
    if (transaction.getReferenceId().has_value()) {
        obj["reference_id"] = Core::Json::makeString(*transaction.getReferenceId());
    }
    if (transaction.getNotes().has_value()) {
        obj["notes"] = Core::Json::makeString(*transaction.getNotes());
    }
    if (transaction.getCreatedAt().has_value()) {
        obj["created_at"] = Core::Json::makeString(*transaction.getCreatedAt());
    }
    
    return json;
}

Response InventoryController::recordTransaction(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[InventoryController] recordTransaction called");
        
        // Parse request body
        auto bodyJson = req.getJson();
        
        if (!bodyJson || !bodyJson->isObject()) {
            LOG_ERROR("[InventoryController] Invalid JSON body");
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();
        LOG_DEBUG("[InventoryController] JSON parsed successfully");
        
        // Montar DTO
        RecordTransactionDto dto;
        dto.tenantId = getTenantId(req);
        dto.createdBy = getUserId(req);
        
        if (bodyObj.count("product_id") && bodyObj["product_id"]->isString()) {
            dto.productId = bodyObj["product_id"]->asString();
        }
        if (bodyObj.count("type") && bodyObj["type"]->isString()) {
            dto.type = bodyObj["type"]->asString();
        }
        if (bodyObj.count("quantity") && bodyObj["quantity"]->isNumber()) {
            dto.quantity = static_cast<int>(bodyObj["quantity"]->asNumber());
        }
        if (bodyObj.count("location_id") && bodyObj["location_id"]->isString()) {
            dto.locationId = bodyObj["location_id"]->asString();
        }
        if (bodyObj.count("reference_type") && bodyObj["reference_type"]->isString()) {
            dto.referenceType = bodyObj["reference_type"]->asString();
        }
        if (bodyObj.count("reference_id") && bodyObj["reference_id"]->isString()) {
            dto.referenceId = bodyObj["reference_id"]->asString();
        }
        if (bodyObj.count("notes") && bodyObj["notes"]->isString()) {
            dto.notes = bodyObj["notes"]->asString();
        }
        
        LOG_DEBUG("[InventoryController] Calling RecordTransactionUseCase - type: " + dto.type + 
            ", quantity: " + std::to_string(dto.quantity));
        
        // Executar use case
        RecordTransactionUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        LOG_DEBUG("[InventoryController] UseCase result - success: " + std::string(result.success ? "true" : "false"));
        
        if (result.success) {
            auto& respObj = json->asObject();
            respObj["success"] = Core::Json::makeBool(true);
            respObj["transaction_id"] = Core::Json::makeString(result.transactionId);
            respObj["message"] = Core::Json::makeString(result.message);
            respObj["new_stock"] = Core::Json::makeNumber(result.newStock);
            return Response(StatusCode::Created).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[InventoryController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response InventoryController::getHistory(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[InventoryController] getHistory called");
        
        GetInventoryHistoryDto dto;
        dto.tenantId = getTenantId(req);
        dto.productId = getQueryParam(req, "product_id");
        dto.locationId = getQueryParam(req, "location_id");
        
        std::string limitParam = getQueryParam(req, "limit");
        if (!limitParam.empty()) {
            dto.limit = std::stoi(limitParam);
        }
        
        LOG_DEBUG("[InventoryController] Query params - product: " + dto.productId + 
            ", location: " + dto.locationId + ", limit: " + std::to_string(dto.limit));
        
        GetInventoryHistoryUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (result.success) {
            auto transactionsArray = Core::Json::makeArray();
            for (const auto& transaction : result.transactions) {
                transactionsArray->asArray().push_back(transactionToJson(transaction));
            }
            
            auto& respObj = json->asObject();
            respObj["transactions"] = transactionsArray;
            respObj["total"] = Core::Json::makeNumber(result.transactions.size());
            
            LOG_DEBUG("[InventoryController] Returning " + std::to_string(result.transactions.size()) + " transactions");
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::InternalServerError).json(*json);
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[InventoryController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response InventoryController::getCurrentStock(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[InventoryController] getCurrentStock called");
        
        std::string productId = req.getParam("productId");
        LOG_DEBUG("[InventoryController] product_id: " + productId);
        
        if (productId.empty()) {
            LOG_ERROR("[InventoryController] Product ID is empty");
            json->asObject()["error"] = Core::Json::makeString("Product ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        GetCurrentStockDto dto;
        dto.tenantId = getTenantId(req);
        dto.productId = productId;
        dto.locationId = getQueryParam(req, "location_id");
        
        LOG_DEBUG("[InventoryController] Calling GetCurrentStockUseCase");
        
        GetCurrentStockUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (result.success) {
            auto& respObj = json->asObject();
            respObj["product_id"] = Core::Json::makeString(dto.productId);
            respObj["stock"] = Core::Json::makeNumber(result.stock);
            
            if (!dto.locationId.empty()) {
                respObj["location_id"] = Core::Json::makeString(dto.locationId);
            }
            
            LOG_DEBUG("[InventoryController] Current stock: " + std::to_string(result.stock));
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[InventoryController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

} // namespace Inventory
} // namespace Domain

