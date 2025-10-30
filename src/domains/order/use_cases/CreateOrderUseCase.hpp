#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../repositories/OrderRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include "../../../core/utils/GlobalMetrics.hpp"

namespace Domain {
namespace Order {

struct CreateOrderItemDto {
    std::string productId;
    std::string productName;
    std::string productCode;
    double quantity;
    double unitPrice;
    double discount = 0;
    double tax = 0;
};

struct CreateOrderDto {
    std::string tenantId;
    std::string customerId;
    std::string userId;
    std::vector<CreateOrderItemDto> items;
    
    std::string paymentMethod;
    double shippingCost = 0;
    double discount = 0;
    double tax = 0;
    
    std::string shippingAddress;
    std::string shippingCity;
    std::string shippingState;
    std::string shippingZipCode;
    std::string customerNotes;
};

struct CreateOrderResult {
    bool success;
    std::string message;
    std::string orderId;
    std::string orderNumber;
};

class CreateOrderUseCase {
public:
    explicit CreateOrderUseCase(std::shared_ptr<OrderRepository> repository)
        : repository_(repository) {}
    
    CreateOrderResult execute(const CreateOrderDto& dto) {
        try {
            if (dto.items.empty()) {
                return {false, "Order must have at least one item", "", ""};
            }
            
            // Generate IDs
            std::string orderId = generateSimpleUUID();
            std::string orderNumber = "ORD-" + std::to_string(std::time(nullptr)) + "-" + orderId.substr(0, 8);
            
            // Create order
            Order order(orderId, dto.tenantId, dto.customerId, dto.userId, orderNumber);
            
            if (!dto.paymentMethod.empty()) {
                order.setPaymentMethod(PaymentMethod(dto.paymentMethod));
            }
            
            order.setShippingCost(dto.shippingCost);
            order.setDiscount(dto.discount);
            order.setTax(dto.tax);
            
            if (!dto.shippingAddress.empty()) order.setShippingAddress(dto.shippingAddress);
            if (!dto.shippingCity.empty()) order.setShippingCity(dto.shippingCity);
            if (!dto.shippingState.empty()) order.setShippingState(dto.shippingState);
            if (!dto.shippingZipCode.empty()) order.setShippingZipCode(dto.shippingZipCode);
            if (!dto.customerNotes.empty()) order.setCustomerNotes(dto.customerNotes);
            
            // Create items
            std::vector<OrderItem> items;
            double subtotal = 0;
            
            for (const auto& itemDto : dto.items) {
                std::string itemId = generateSimpleUUID();
                OrderItem item(itemId, orderId, itemDto.productId, itemDto.productName,
                             itemDto.quantity, itemDto.unitPrice);
                
                if (!itemDto.productCode.empty()) item.setProductCode(itemDto.productCode);
                item.setDiscount(itemDto.discount);
                item.setTax(itemDto.tax);
                
                subtotal += item.getSubtotal();
                items.push_back(item);
            }
            
            order.setSubtotal(subtotal);
            
            // Save
            if (!repository_->save(order)) {
                return {false, "Failed to save order", "", ""};
            }
            
            if (!repository_->saveItems(items)) {
                return {false, "Failed to save order items", "", ""};
            }
            
            LOG_INFO("[CreateOrderUseCase] Order created: " + orderNumber);
            
            // Metrics: Track order creation
            GlobalMetrics::recordOrderCreated(dto.tenantId, subtotal + order.getShippingCost() + order.getTax());
            
            return {true, "Order created successfully", orderId, orderNumber};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[CreateOrderUseCase] Error: " + std::string(e.what()));
            return {false, std::string("Error: ") + e.what(), "", ""};
        }
    }
    
private:
    std::shared_ptr<OrderRepository> repository_;
    
    std::string generateSimpleUUID() {
        static const char* chars = "0123456789abcdef";
        std::string uuid;
        uuid.reserve(36);
        
        std::srand(std::time(nullptr) + std::rand());
        
        for (int i = 0; i < 36; i++) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                uuid += '-';
            } else {
                uuid += chars[std::rand() % 16];
            }
        }
        
        return uuid;
    }
};

} // namespace Order
} // namespace Domain

