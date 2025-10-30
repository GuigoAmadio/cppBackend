#include "CategoryController.hpp"
#include "../../../core/utils/LoggerNew.hpp"

using namespace Core::Http;

namespace Domain {
namespace Category {

Response CategoryController::createCategory(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[CategoryController] createCategory called");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto& bodyObj = bodyJson->asObject();
        
        std::string userId = req.getCustomData("user_id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        if (tenantId.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Tenant ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        CreateCategoryDto dto;
        dto.tenantId = tenantId;
        dto.createdBy = userId;
        
        if (bodyObj.find("name") != bodyObj.end() && bodyObj["name"]->isString()) {
            dto.name = bodyObj["name"]->asString();
        }
        if (bodyObj.find("slug") != bodyObj.end() && bodyObj["slug"]->isString()) {
            dto.slug = bodyObj["slug"]->asString();
        }
        if (bodyObj.find("parent_id") != bodyObj.end() && bodyObj["parent_id"]->isString()) {
            dto.parentId = bodyObj["parent_id"]->asString();
        }
        if (bodyObj.find("description") != bodyObj.end() && bodyObj["description"]->isString()) {
            dto.description = bodyObj["description"]->asString();
        }
        if (bodyObj.find("image_url") != bodyObj.end() && bodyObj["image_url"]->isString()) {
            dto.imageUrl = bodyObj["image_url"]->asString();
        }
        if (bodyObj.find("display_order") != bodyObj.end() && bodyObj["display_order"]->isNumber()) {
            dto.displayOrder = static_cast<int>(bodyObj["display_order"]->asNumber());
        }
        
        CreateCategoryUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success) {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        json->asObject()["id"] = Core::Json::makeString(result.categoryId);
        json->asObject()["message"] = Core::Json::makeString(result.message);
        return Response(StatusCode::Created).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[CategoryController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CategoryController::getCategory(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string categoryId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        GetCategoryDto dto;
        dto.categoryId = categoryId;
        dto.tenantId = tenantId;
        
        GetCategoryUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success || !result.category.has_value()) {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::NotFound).json(*json);
        }
        
        auto& category = result.category.value();
        
        json->asObject()["id"] = Core::Json::makeString(category.getId());
        json->asObject()["tenant_id"] = Core::Json::makeString(category.getTenantId());
        json->asObject()["name"] = Core::Json::makeString(category.getName());
        json->asObject()["slug"] = Core::Json::makeString(category.getSlug());
        json->asObject()["status"] = Core::Json::makeString(category.getStatus().toString());
        
        if (category.getParentId().has_value()) {
            json->asObject()["parent_id"] = Core::Json::makeString(*category.getParentId());
        }
        if (category.getDescription().has_value()) {
            json->asObject()["description"] = Core::Json::makeString(*category.getDescription());
        }
        if (category.getImageUrl().has_value()) {
            json->asObject()["image_url"] = Core::Json::makeString(*category.getImageUrl());
        }
        if (category.getDisplayOrder().has_value()) {
            json->asObject()["display_order"] = Core::Json::makeNumber(*category.getDisplayOrder());
        }
        if (category.getCreatedAt().has_value()) {
            json->asObject()["created_at"] = Core::Json::makeString(*category.getCreatedAt());
        }
        
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CategoryController::listCategories(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string limitStr = getQueryParam(req, "limit");
        std::string offsetStr = getQueryParam(req, "offset");
        
        ListCategoriesDto dto;
        dto.tenantId = tenantId;
        dto.limit = limitStr.empty() ? 100 : std::stoi(limitStr);
        dto.offset = offsetStr.empty() ? 0 : std::stoi(offsetStr);
        
        ListCategoriesUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success) {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto categoriesArray = Core::Json::makeArray();
        for (const auto& category : result.categories) {
            auto catObj = Core::Json::makeObject();
            catObj->asObject()["id"] = Core::Json::makeString(category.getId());
            catObj->asObject()["name"] = Core::Json::makeString(category.getName());
            catObj->asObject()["slug"] = Core::Json::makeString(category.getSlug());
            catObj->asObject()["status"] = Core::Json::makeString(category.getStatus().toString());
            
            if (category.getParentId().has_value()) {
                catObj->asObject()["parent_id"] = Core::Json::makeString(*category.getParentId());
            }
            if (category.getDisplayOrder().has_value()) {
                catObj->asObject()["display_order"] = Core::Json::makeNumber(*category.getDisplayOrder());
            }
            
            categoriesArray->asArray().push_back(std::move(catObj));
        }
        
        json->asObject()["categories"] = std::move(categoriesArray);
        json->asObject()["total"] = Core::Json::makeNumber(result.total);
        
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CategoryController::getCategoryChildren(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string categoryId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        ListCategoriesDto dto;
        dto.tenantId = tenantId;
        dto.parentId = categoryId;
        
        ListCategoriesUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        auto childrenArray = Core::Json::makeArray();
        for (const auto& category : result.categories) {
            auto catObj = Core::Json::makeObject();
            catObj->asObject()["id"] = Core::Json::makeString(category.getId());
            catObj->asObject()["name"] = Core::Json::makeString(category.getName());
            catObj->asObject()["slug"] = Core::Json::makeString(category.getSlug());
            catObj->asObject()["status"] = Core::Json::makeString(category.getStatus().toString());
            childrenArray->asArray().push_back(std::move(catObj));
        }
        
        json->asObject()["children"] = std::move(childrenArray);
        json->asObject()["total"] = Core::Json::makeNumber(result.total);
        
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CategoryController::getRootCategories(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        ListCategoriesDto dto;
        dto.tenantId = tenantId;
        dto.parentId = std::nullopt;  // Root categories
        
        ListCategoriesUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        auto categoriesArray = Core::Json::makeArray();
        for (const auto& category : result.categories) {
            auto catObj = Core::Json::makeObject();
            catObj->asObject()["id"] = Core::Json::makeString(category.getId());
            catObj->asObject()["name"] = Core::Json::makeString(category.getName());
            catObj->asObject()["slug"] = Core::Json::makeString(category.getSlug());
            catObj->asObject()["status"] = Core::Json::makeString(category.getStatus().toString());
            categoriesArray->asArray().push_back(std::move(catObj));
        }
        
        json->asObject()["categories"] = std::move(categoriesArray);
        json->asObject()["total"] = Core::Json::makeNumber(result.total);
        
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CategoryController::updateCategory(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string categoryId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        LOG_DEBUG("[CategoryController] updateCategory - ID: " + categoryId + ", Tenant: " + tenantId);
        LOG_DEBUG("[CategoryController] Raw Body: " + req.getBody());
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            LOG_WARNING("[CategoryController] Invalid JSON body - Raw: " + req.getBody());
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto& bodyObj = bodyJson->asObject();
        
        UpdateCategoryDto dto;
        dto.categoryId = categoryId;
        dto.tenantId = tenantId;
        
        if (bodyObj.find("name") != bodyObj.end() && bodyObj["name"]->isString()) {
            dto.name = bodyObj["name"]->asString();
        }
        if (bodyObj.find("slug") != bodyObj.end() && bodyObj["slug"]->isString()) {
            dto.slug = bodyObj["slug"]->asString();
        }
        if (bodyObj.find("description") != bodyObj.end() && bodyObj["description"]->isString()) {
            dto.description = bodyObj["description"]->asString();
        }
        if (bodyObj.find("image_url") != bodyObj.end() && bodyObj["image_url"]->isString()) {
            dto.imageUrl = bodyObj["image_url"]->asString();
        }
        if (bodyObj.find("display_order") != bodyObj.end() && bodyObj["display_order"]->isNumber()) {
            dto.displayOrder = static_cast<int>(bodyObj["display_order"]->asNumber());
        }
        if (bodyObj.find("status") != bodyObj.end() && bodyObj["status"]->isString()) {
            dto.status = bodyObj["status"]->asString();
        }
        if (bodyObj.find("parent_id") != bodyObj.end() && bodyObj["parent_id"]->isString()) {
            dto.parentId = bodyObj["parent_id"]->asString();
        }
        
        UpdateCategoryUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success) {
            LOG_WARNING("[CategoryController] Update failed: " + result.message);
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        LOG_INFO("[CategoryController] Category updated successfully: " + categoryId);
        
        json->asObject()["message"] = Core::Json::makeString(result.message);
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CategoryController::deleteCategory(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string categoryId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        DeleteCategoryDto dto;
        dto.categoryId = categoryId;
        dto.tenantId = tenantId;
        
        DeleteCategoryUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success) {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        json->asObject()["message"] = Core::Json::makeString(result.message);
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

} // namespace Category
} // namespace Domain

