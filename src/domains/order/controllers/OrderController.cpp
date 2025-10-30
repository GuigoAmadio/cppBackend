#include "OrderController.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include "../use_cases/UpdateOrderStatusUseCase.hpp"
#include "../use_cases/UpdatePaymentStatusUseCase.hpp"
#include "../use_cases/CancelOrderUseCase.hpp"
#include "../use_cases/ListCustomerOrdersUseCase.hpp"

using namespace Core::Http;

namespace Domain {
namespace Order {

Response OrderController::createOrder(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_DEBUG("[OrderController] createOrder called");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            LOG_WARNING("[OrderController] Invalid JSON body");
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
        
        CreateOrderDto dto;
        dto.tenantId = tenantId;
        dto.userId = userId;
        
        // Parse customer_id
        if (bodyObj.find("customer_id") != bodyObj.end() && bodyObj["customer_id"]->isString()) {
            dto.customerId = bodyObj["customer_id"]->asString();
        } else {
            json->asObject()["error"] = Core::Json::makeString("customer_id is required");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        // Parse items
        if (bodyObj.find("items") != bodyObj.end() && bodyObj["items"]->isArray()) {
            auto& itemsArray = bodyObj["items"]->asArray();
            for (const auto& itemJson : itemsArray) {
                if (!itemJson->isObject()) continue;
                
                auto& itemObj = itemJson->asObject();
                CreateOrderItemDto item;
                
                if (itemObj.find("product_id") != itemObj.end() && itemObj["product_id"]->isString()) {
                    item.productId = itemObj["product_id"]->asString();
                }
                if (itemObj.find("product_name") != itemObj.end() && itemObj["product_name"]->isString()) {
                    item.productName = itemObj["product_name"]->asString();
                }
                if (itemObj.find("product_code") != itemObj.end() && itemObj["product_code"]->isString()) {
                    item.productCode = itemObj["product_code"]->asString();
                }
                if (itemObj.find("quantity") != itemObj.end() && itemObj["quantity"]->isNumber()) {
                    item.quantity = itemObj["quantity"]->asNumber();
                }
                if (itemObj.find("unit_price") != itemObj.end() && itemObj["unit_price"]->isNumber()) {
                    item.unitPrice = itemObj["unit_price"]->asNumber();
                }
                if (itemObj.find("discount") != itemObj.end() && itemObj["discount"]->isNumber()) {
                    item.discount = itemObj["discount"]->asNumber();
                }
                if (itemObj.find("tax") != itemObj.end() && itemObj["tax"]->isNumber()) {
                    item.tax = itemObj["tax"]->asNumber();
                }
                
                dto.items.push_back(item);
            }
        }
        
        // Parse optional fields
        if (bodyObj.find("payment_method") != bodyObj.end() && bodyObj["payment_method"]->isString()) {
            dto.paymentMethod = bodyObj["payment_method"]->asString();
        }
        if (bodyObj.find("shipping_cost") != bodyObj.end() && bodyObj["shipping_cost"]->isNumber()) {
            dto.shippingCost = bodyObj["shipping_cost"]->asNumber();
        }
        if (bodyObj.find("discount") != bodyObj.end() && bodyObj["discount"]->isNumber()) {
            dto.discount = bodyObj["discount"]->asNumber();
        }
        if (bodyObj.find("tax") != bodyObj.end() && bodyObj["tax"]->isNumber()) {
            dto.tax = bodyObj["tax"]->asNumber();
        }
        if (bodyObj.find("shipping_address") != bodyObj.end() && bodyObj["shipping_address"]->isString()) {
            dto.shippingAddress = bodyObj["shipping_address"]->asString();
        }
        if (bodyObj.find("shipping_city") != bodyObj.end() && bodyObj["shipping_city"]->isString()) {
            dto.shippingCity = bodyObj["shipping_city"]->asString();
        }
        if (bodyObj.find("shipping_state") != bodyObj.end() && bodyObj["shipping_state"]->isString()) {
            dto.shippingState = bodyObj["shipping_state"]->asString();
        }
        if (bodyObj.find("shipping_zip_code") != bodyObj.end() && bodyObj["shipping_zip_code"]->isString()) {
            dto.shippingZipCode = bodyObj["shipping_zip_code"]->asString();
        }
        if (bodyObj.find("customer_notes") != bodyObj.end() && bodyObj["customer_notes"]->isString()) {
            dto.customerNotes = bodyObj["customer_notes"]->asString();
        }
        
        CreateOrderUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success) {
            LOG_WARNING("[OrderController] Failed to create order: " + result.message);
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        json->asObject()["id"] = Core::Json::makeString(result.orderId);  // Padronizado para "id"
        json->asObject()["order_number"] = Core::Json::makeString(result.orderNumber);
        json->asObject()["message"] = Core::Json::makeString(result.message);
        return Response(StatusCode::Created).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[OrderController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response OrderController::getOrder(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string orderId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        LOG_DEBUG("[OrderController] getOrder - ID: " + orderId);
        
        GetOrderDto dto;
        dto.orderId = orderId;
        dto.tenantId = tenantId;
        
        GetOrderUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        if (!result.success || !result.order.has_value()) {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::NotFound).json(*json);
        }
        
        auto& order = result.order.value();
        
        // Order data
        json->asObject()["id"] = Core::Json::makeString(order.getId());
        json->asObject()["order_number"] = Core::Json::makeString(order.getOrderNumber());
        json->asObject()["customer_id"] = Core::Json::makeString(order.getCustomerId());
        json->asObject()["status"] = Core::Json::makeString(order.getStatus().toString());
        json->asObject()["payment_status"] = Core::Json::makeString(order.getPaymentStatus().toString());
        
        if (order.getPaymentMethod().has_value()) {
            json->asObject()["payment_method"] = Core::Json::makeString(order.getPaymentMethod()->toString());
        }
        
        json->asObject()["subtotal"] = Core::Json::makeNumber(order.getSubtotal());
        json->asObject()["discount"] = Core::Json::makeNumber(order.getDiscount());
        json->asObject()["tax"] = Core::Json::makeNumber(order.getTax());
        json->asObject()["shipping_cost"] = Core::Json::makeNumber(order.getShippingCost());
        json->asObject()["total"] = Core::Json::makeNumber(order.getTotal());
        
        if (order.getCreatedAt().has_value()) {
            json->asObject()["created_at"] = Core::Json::makeString(*order.getCreatedAt());
        }
        
        // Items
        auto itemsArray = Core::Json::makeArray();
        for (const auto& item : order.getItems()) {
            auto itemObj = Core::Json::makeObject();
            itemObj->asObject()["id"] = Core::Json::makeString(item.getId());
            itemObj->asObject()["product_id"] = Core::Json::makeString(item.getProductId());
            itemObj->asObject()["product_name"] = Core::Json::makeString(item.getProductName());
            itemObj->asObject()["quantity"] = Core::Json::makeNumber(item.getQuantity());
            itemObj->asObject()["unit_price"] = Core::Json::makeNumber(item.getUnitPrice());
            itemObj->asObject()["subtotal"] = Core::Json::makeNumber(item.getSubtotal());
            itemObj->asObject()["total"] = Core::Json::makeNumber(item.getTotal());
            itemsArray->asArray().push_back(std::move(itemObj));
        }
        json->asObject()["items"] = std::move(itemsArray);
        
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[OrderController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response OrderController::listOrders(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string limitStr = getQueryParam(req, "limit");
        std::string offsetStr = getQueryParam(req, "offset");
        
        ListOrdersDto dto;
        dto.tenantId = tenantId;
        dto.limit = limitStr.empty() ? 50 : std::stoi(limitStr);
        dto.offset = offsetStr.empty() ? 0 : std::stoi(offsetStr);
        
        ListOrdersUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        auto ordersArray = Core::Json::makeArray();
        for (const auto& order : result.orders) {
            auto orderObj = Core::Json::makeObject();
            orderObj->asObject()["id"] = Core::Json::makeString(order.getId());
            orderObj->asObject()["order_number"] = Core::Json::makeString(order.getOrderNumber());
            orderObj->asObject()["customer_id"] = Core::Json::makeString(order.getCustomerId());
            orderObj->asObject()["status"] = Core::Json::makeString(order.getStatus().toString());
            orderObj->asObject()["payment_status"] = Core::Json::makeString(order.getPaymentStatus().toString());
            orderObj->asObject()["total"] = Core::Json::makeNumber(order.getTotal());
            if (order.getCreatedAt().has_value()) {
                orderObj->asObject()["created_at"] = Core::Json::makeString(*order.getCreatedAt());
            }
            ordersArray->asArray().push_back(std::move(orderObj));
        }
        
        json->asObject()["orders"] = std::move(ordersArray);
        json->asObject()["total"] = Core::Json::makeNumber(result.total);
        
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[OrderController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response OrderController::updateStatus(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string orderId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto& bodyObj = bodyJson->asObject();
        
        UpdateOrderStatusDto dto;
        dto.orderId = orderId;
        dto.tenantId = tenantId;
        
        if (bodyObj.find("status") != bodyObj.end() && bodyObj["status"]->isString()) {
            dto.status = bodyObj["status"]->asString();
        } else {
            json->asObject()["error"] = Core::Json::makeString("status is required");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        UpdateOrderStatusUseCase useCase(repository_);
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

Response OrderController::updatePaymentStatus(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string orderId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto& bodyObj = bodyJson->asObject();
        
        UpdatePaymentStatusDto dto;
        dto.orderId = orderId;
        dto.tenantId = tenantId;
        
        if (bodyObj.find("payment_status") != bodyObj.end() && bodyObj["payment_status"]->isString()) {
            dto.paymentStatus = bodyObj["payment_status"]->asString();
        } else {
            json->asObject()["error"] = Core::Json::makeString("payment_status is required");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        UpdatePaymentStatusUseCase useCase(repository_);
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

Response OrderController::cancelOrder(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string orderId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        auto bodyJson = req.getJson();
        std::string reason;
        if (bodyJson && bodyJson->isObject()) {
            auto& bodyObj = bodyJson->asObject();
            if (bodyObj.find("reason") != bodyObj.end() && bodyObj["reason"]->isString()) {
                reason = bodyObj["reason"]->asString();
            }
        }
        
        CancelOrderDto dto;
        dto.orderId = orderId;
        dto.tenantId = tenantId;
        dto.reason = reason;
        
        CancelOrderUseCase useCase(repository_);
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

Response OrderController::listCustomerOrders(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string customerId = req.getParam("customerId");
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string limitStr = getQueryParam(req, "limit");
        
        ListCustomerOrdersDto dto;
        dto.customerId = customerId;
        dto.tenantId = tenantId;
        dto.limit = limitStr.empty() ? 50 : std::stoi(limitStr);
        
        ListCustomerOrdersUseCase useCase(repository_);
        auto result = useCase.execute(dto);
        
        auto ordersArray = Core::Json::makeArray();
        for (const auto& order : result.orders) {
            auto orderObj = Core::Json::makeObject();
            orderObj->asObject()["id"] = Core::Json::makeString(order.getId());
            orderObj->asObject()["order_number"] = Core::Json::makeString(order.getOrderNumber());
            orderObj->asObject()["status"] = Core::Json::makeString(order.getStatus().toString());
            orderObj->asObject()["total"] = Core::Json::makeNumber(order.getTotal());
            ordersArray->asArray().push_back(std::move(orderObj));
        }
        
        json->asObject()["orders"] = std::move(ordersArray);
        json->asObject()["total"] = Core::Json::makeNumber(result.total);
        
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(std::string("Error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

} // namespace Order
} // namespace Domain

