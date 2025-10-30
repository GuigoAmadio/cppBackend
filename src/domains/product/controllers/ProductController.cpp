#include "ProductController.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>

namespace Domain {
namespace Product {

std::shared_ptr<Core::Json::JsonValue> ProductController::productToJson(const Product& product) {
    auto json = Core::Json::makeObject();
    auto& obj = json->asObject();

    obj["id"] = Core::Json::makeString(product.getId());
    obj["tenant_id"] = Core::Json::makeString(product.getTenantId());
    obj["code"] = Core::Json::makeString(product.getCode().toString());
    obj["name"] = Core::Json::makeString(product.getName());
    obj["type"] = Core::Json::makeString(product.getType().toString());
    obj["status"] = Core::Json::makeString(product.getStatus().toString());
    obj["price"] = Core::Json::makeNumber(product.getPrice().toDecimal());
    obj["cost"] = Core::Json::makeNumber(product.getCost().toDecimal());
    obj["currency"] = Core::Json::makeString(product.getPrice().currency());
    obj["track_inventory"] = Core::Json::makeBool(product.trackInventory());
    obj["stock_quantity"] = Core::Json::makeNumber(product.getStock().quantity());
    obj["low_stock_threshold"] = Core::Json::makeNumber(product.getStock().lowStockThreshold());
    obj["is_available"] = Core::Json::makeBool(product.isAvailable());
    obj["is_low_stock"] = Core::Json::makeBool(product.isLowStock());

    if (product.getWorkspaceId().has_value()) {
        obj["workspace_id"] = Core::Json::makeString(*product.getWorkspaceId());
    }
    if (product.getCategoryId().has_value()) {
        obj["category_id"] = Core::Json::makeString(*product.getCategoryId());
    }
    if (product.getDescription().has_value()) {
        obj["description"] = Core::Json::makeString(*product.getDescription());
    }
    if (product.getSku().has_value()) {
        obj["sku"] = Core::Json::makeString(*product.getSku());
    }
    if (product.getBarcode().has_value()) {
        obj["barcode"] = Core::Json::makeString(*product.getBarcode());
    }
    if (product.getWeight().has_value()) {
        obj["weight"] = Core::Json::makeNumber(*product.getWeight());
        if (product.getWeightUnit().has_value()) {
            obj["weight_unit"] = Core::Json::makeString(*product.getWeightUnit());
        }
    }
    if (product.getCreatedAt().has_value()) {
        obj["created_at"] = Core::Json::makeString(*product.getCreatedAt());
    }
    if (product.getUpdatedAt().has_value()) {
        obj["updated_at"] = Core::Json::makeString(*product.getUpdatedAt());
    }

    return json;
}

Response ProductController::createProduct(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[ProductController] createProduct called");
        
        // Parse request body
        auto bodyJson = req.getJson();
        
        if (!bodyJson || !bodyJson->isObject()) {
            LOG_ERROR("[ProductController] Invalid JSON body");
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();
        LOG_DEBUG("[ProductController] JSON parsed successfully");

        // Montar DTO
        CreateProductDto dto;
        dto.tenantId = getTenantId(req);
        dto.createdBy = getUserId(req);

        // Campos obrigatórios
        if (bodyObj.count("code") && bodyObj["code"]->isString()) {
            dto.code = bodyObj["code"]->asString();
        } else {
            json->asObject()["error"] = Core::Json::makeString("Field 'code' is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        if (bodyObj.count("name") && bodyObj["name"]->isString()) {
            dto.name = bodyObj["name"]->asString();
        } else {
            json->asObject()["error"] = Core::Json::makeString("Field 'name' is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        if (bodyObj.count("type") && bodyObj["type"]->isString()) {
            dto.type = bodyObj["type"]->asString();
        } else {
            dto.type = "physical"; // default
        }

        if (bodyObj.count("price") && bodyObj["price"]->isNumber()) {
            dto.price = bodyObj["price"]->asNumber();
        } else {
            json->asObject()["error"] = Core::Json::makeString("Field 'price' is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Campos opcionais
        if (bodyObj.count("workspace_id") && bodyObj["workspace_id"]->isString()) {
            dto.workspaceId = bodyObj["workspace_id"]->asString();
        }
        if (bodyObj.count("category_id") && bodyObj["category_id"]->isString()) {
            dto.categoryId = bodyObj["category_id"]->asString();
        }
        if (bodyObj.count("description") && bodyObj["description"]->isString()) {
            dto.description = bodyObj["description"]->asString();
        }
        if (bodyObj.count("currency") && bodyObj["currency"]->isString()) {
            dto.currency = bodyObj["currency"]->asString();
        }
        if (bodyObj.count("cost") && bodyObj["cost"]->isNumber()) {
            dto.cost = bodyObj["cost"]->asNumber();
        }
        if (bodyObj.count("track_inventory") && bodyObj["track_inventory"]->isBool()) {
            dto.trackInventory = bodyObj["track_inventory"]->asBool();
        }
        if (bodyObj.count("stock_quantity") && bodyObj["stock_quantity"]->isNumber()) {
            dto.stockQuantity = static_cast<int>(bodyObj["stock_quantity"]->asNumber());
        }
        if (bodyObj.count("low_stock_threshold") && bodyObj["low_stock_threshold"]->isNumber()) {
            dto.lowStockThreshold = static_cast<int>(bodyObj["low_stock_threshold"]->asNumber());
        }
        if (bodyObj.count("sku") && bodyObj["sku"]->isString()) {
            dto.sku = bodyObj["sku"]->asString();
        }
        if (bodyObj.count("barcode") && bodyObj["barcode"]->isString()) {
            dto.barcode = bodyObj["barcode"]->asString();
        }
        if (bodyObj.count("weight") && bodyObj["weight"]->isNumber()) {
            dto.weight = bodyObj["weight"]->asNumber();
        }
        if (bodyObj.count("weight_unit") && bodyObj["weight_unit"]->isString()) {
            dto.weightUnit = bodyObj["weight_unit"]->asString();
        }

        // Executar use case
        LOG_DEBUG("[ProductController] Calling CreateProductUseCase - code: " + dto.code + ", name: " + dto.name);
        CreateProductUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        LOG_DEBUG("[ProductController] UseCase result - success: " + std::string(result.success ? "true" : "false"));
        if (result.success) {
            auto& respObj = json->asObject();
            respObj["success"] = Core::Json::makeBool(true);
            respObj["id"] = Core::Json::makeString(result.productId);  // Padronizado para "id"
            respObj["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::Created).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error creating product: ") + e.what());
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response ProductController::getProduct(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string productId = req.getParam("id");
        std::string tenantId = getTenantId(req);

        GetProductUseCase useCase(repository_);
        auto result = useCase.execute({productId, tenantId});

        if (result.success && result.product.has_value()) {
            json->asObject()["product"] = productToJson(*result.product);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::NotFound).json(*json);
        }

    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error getting product: ") + e.what());
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response ProductController::listProducts(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        ListProductsDto dto;
        dto.tenantId = getTenantId(req);
        
        // Parse query params
        std::string workspaceIdParam = getQueryParam(req, "workspace_id");
        if (!workspaceIdParam.empty()) {
            dto.workspaceId = workspaceIdParam;
        }

        std::string categoryIdParam = getQueryParam(req, "category_id");
        if (!categoryIdParam.empty()) {
            dto.categoryId = categoryIdParam;
        }

        std::string statusParam = getQueryParam(req, "status");
        if (!statusParam.empty()) {
            dto.status = statusParam;
        }

        std::string limitParam = getQueryParam(req, "limit");
        if (!limitParam.empty()) {
            dto.limit = std::stoi(limitParam);
        }

        std::string offsetParam = getQueryParam(req, "offset");
        if (!offsetParam.empty()) {
            dto.offset = std::stoi(offsetParam);
        }

        ListProductsUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        if (result.success) {
            auto productsArray = Core::Json::makeArray();
            for (const auto& product : result.products) {
                productsArray->asArray().push_back(productToJson(product));
            }

            auto& respObj = json->asObject();
            respObj["products"] = productsArray;
            respObj["total"] = Core::Json::makeNumber(result.total);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::InternalServerError).json(*json);
        }

    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error listing products: ") + e.what());
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response ProductController::updateProduct(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string productId = req.getParam("id");
        auto bodyJson = req.getJson();
        
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        UpdateProductDto dto;
        dto.productId = productId;
        dto.tenantId = getTenantId(req);

        // Parse campos opcionais
        if (bodyObj.count("name") && bodyObj["name"]->isString()) {
            dto.name = bodyObj["name"]->asString();
        }
        if (bodyObj.count("description") && bodyObj["description"]->isString()) {
            dto.description = bodyObj["description"]->asString();
        }
        if (bodyObj.count("status") && bodyObj["status"]->isString()) {
            dto.status = bodyObj["status"]->asString();
        }
        if (bodyObj.count("price") && bodyObj["price"]->isNumber()) {
            dto.price = bodyObj["price"]->asNumber();
        }
        if (bodyObj.count("cost") && bodyObj["cost"]->isNumber()) {
            dto.cost = bodyObj["cost"]->asNumber();
        }
        if (bodyObj.count("currency") && bodyObj["currency"]->isString()) {
            dto.currency = bodyObj["currency"]->asString();
        }
        if (bodyObj.count("track_inventory") && bodyObj["track_inventory"]->isBool()) {
            dto.trackInventory = bodyObj["track_inventory"]->asBool();
        }
        if (bodyObj.count("stock_quantity") && bodyObj["stock_quantity"]->isNumber()) {
            dto.stockQuantity = static_cast<int>(bodyObj["stock_quantity"]->asNumber());
        }
        if (bodyObj.count("low_stock_threshold") && bodyObj["low_stock_threshold"]->isNumber()) {
            dto.lowStockThreshold = static_cast<int>(bodyObj["low_stock_threshold"]->asNumber());
        }
        if (bodyObj.count("sku") && bodyObj["sku"]->isString()) {
            dto.sku = bodyObj["sku"]->asString();
        }
        if (bodyObj.count("barcode") && bodyObj["barcode"]->isString()) {
            dto.barcode = bodyObj["barcode"]->asString();
        }
        if (bodyObj.count("weight") && bodyObj["weight"]->isNumber()) {
            dto.weight = bodyObj["weight"]->asNumber();
        }
        if (bodyObj.count("weight_unit") && bodyObj["weight_unit"]->isString()) {
            dto.weightUnit = bodyObj["weight_unit"]->asString();
        }

        UpdateProductUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        if (result.success) {
            auto& respObj = json->asObject();
            respObj["success"] = Core::Json::makeBool(true);
            respObj["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error updating product: ") + e.what());
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response ProductController::deleteProduct(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string productId = req.getParam("id");
        std::string hardDelete = getQueryParam(req, "hard");

        DeleteProductDto dto;
        dto.productId = productId;
        dto.tenantId = getTenantId(req);
        dto.softDelete = (hardDelete != "true"); // Por padrão, soft delete

        DeleteProductUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        if (result.success) {
            auto& respObj = json->asObject();
            respObj["success"] = Core::Json::makeBool(true);
            respObj["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::NotFound).json(*json);
        }

    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error deleting product: ") + e.what());
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response ProductController::listLowStock(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        ListProductsDto dto;
        dto.tenantId = getTenantId(req);
        dto.onlyLowStock = true;

        std::string limitParam = getQueryParam(req, "limit");
        if (!limitParam.empty()) {
            dto.limit = std::stoi(limitParam);
        }

        ListProductsUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        if (result.success) {
            auto productsArray = Core::Json::makeArray();
            for (const auto& product : result.products) {
                productsArray->asArray().push_back(productToJson(product));
            }

            auto& respObj = json->asObject();
            respObj["products"] = productsArray;
            respObj["total"] = Core::Json::makeNumber(result.total);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::InternalServerError).json(*json);
        }

    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error listing low stock products: ") + e.what());
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response ProductController::searchProducts(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[ProductController] searchProducts called");
        LOG_DEBUG("[ProductController] Request path: " + req.getPath());
        
        std::string searchQuery = getQueryParam(req, "q");
        LOG_DEBUG("[ProductController] Search query extracted: '" + searchQuery + "'");
        LOG_DEBUG("[ProductController] Query empty: " + std::string(searchQuery.empty() ? "YES" : "NO"));
        
        if (searchQuery.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Query parameter 'q' is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        ListProductsDto dto;
        dto.tenantId = getTenantId(req);
        dto.searchQuery = searchQuery;

        std::string limitParam = getQueryParam(req, "limit");
        if (!limitParam.empty()) {
            dto.limit = std::stoi(limitParam);
        }

        ListProductsUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        if (result.success) {
            auto productsArray = Core::Json::makeArray();
            for (const auto& product : result.products) {
                productsArray->asArray().push_back(productToJson(product));
            }

            auto& respObj = json->asObject();
            respObj["products"] = productsArray;
            respObj["total"] = Core::Json::makeNumber(result.products.size());
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::InternalServerError).json(*json);
        }

    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error searching products: ") + e.what());
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

} // namespace Product
} // namespace Domain

