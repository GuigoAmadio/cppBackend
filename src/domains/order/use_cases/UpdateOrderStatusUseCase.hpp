#pragma once
#include <memory>
#include "../repositories/OrderRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Order {

struct UpdateOrderStatusDto {
    std::string orderId;
    std::string tenantId;
    std::string status;  // pending, confirmed, processing, shipped, delivered, cancelled
};

struct UpdateOrderStatusResult {
    bool success;
    std::string message;
};

class UpdateOrderStatusUseCase {
public:
    explicit UpdateOrderStatusUseCase(std::shared_ptr<OrderRepository> repository)
        : repository_(repository) {}
    
    UpdateOrderStatusResult execute(const UpdateOrderStatusDto& dto) {
        try {
            LOG_DEBUG("[UpdateOrderStatusUseCase] Updating order: " + dto.orderId + " to status: " + dto.status);
            
            // Validate status
            OrderStatus newStatus;
            try {
                newStatus = OrderStatus(dto.status);
            } catch (const std::exception& e) {
                LOG_WARNING("[UpdateOrderStatusUseCase] Invalid status: " + dto.status);
                return {false, "Invalid status: " + dto.status};
            }
            
            // Get order to validate tenant
            auto orderOpt = repository_->findById(dto.orderId);
            if (!orderOpt.has_value()) {
                LOG_WARNING("[UpdateOrderStatusUseCase] Order not found: " + dto.orderId);
                return {false, "Order not found"};
            }
            
            if (!dto.tenantId.empty() && orderOpt->getTenantId() != dto.tenantId) {
                LOG_WARNING("[UpdateOrderStatusUseCase] Access denied for tenant: " + dto.tenantId);
                return {false, "Access denied"};
            }
            
            // Validate status transition
            auto currentStatus = orderOpt->getStatus();
            if (currentStatus.getValue() == OrderStatus::Value::CANCELLED) {
                return {false, "Cannot update status of cancelled order"};
            }
            if (currentStatus.getValue() == OrderStatus::Value::DELIVERED &&
                newStatus.getValue() != OrderStatus::Value::CANCELLED) {
                return {false, "Cannot change status of delivered order (only cancellation allowed)"};
            }
            
            // Update
            if (!repository_->updateStatus(dto.orderId, newStatus)) {
                LOG_ERROR("[UpdateOrderStatusUseCase] Failed to update order status");
                return {false, "Failed to update order status"};
            }
            
            LOG_INFO("[UpdateOrderStatusUseCase] Order status updated: " + dto.orderId + " -> " + dto.status);
            return {true, "Order status updated successfully"};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[UpdateOrderStatusUseCase] Error: " + std::string(e.what()));
            return {false, std::string("Error: ") + e.what()};
        }
    }
    
private:
    std::shared_ptr<OrderRepository> repository_;
};

} // namespace Order
} // namespace Domain

