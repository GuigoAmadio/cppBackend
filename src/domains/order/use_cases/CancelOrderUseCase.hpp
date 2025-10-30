#pragma once
#include <memory>
#include "../repositories/OrderRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Order {

struct CancelOrderDto {
    std::string orderId;
    std::string tenantId;
    std::string reason;
};

struct CancelOrderResult {
    bool success;
    std::string message;
};

class CancelOrderUseCase {
public:
    explicit CancelOrderUseCase(std::shared_ptr<OrderRepository> repository)
        : repository_(repository) {}
    
    CancelOrderResult execute(const CancelOrderDto& dto) {
        try {
            LOG_DEBUG("[CancelOrderUseCase] Cancelling order: " + dto.orderId);
            
            // Get order
            auto orderOpt = repository_->findById(dto.orderId);
            if (!orderOpt.has_value()) {
                LOG_WARNING("[CancelOrderUseCase] Order not found: " + dto.orderId);
                return {false, "Order not found"};
            }
            
            if (!dto.tenantId.empty() && orderOpt->getTenantId() != dto.tenantId) {
                LOG_WARNING("[CancelOrderUseCase] Access denied for tenant: " + dto.tenantId);
                return {false, "Access denied"};
            }
            
            // Validate cancellation
            auto currentStatus = orderOpt->getStatus();
            if (currentStatus.getValue() == OrderStatus::Value::CANCELLED) {
                return {false, "Order is already cancelled"};
            }
            
            if (currentStatus.getValue() == OrderStatus::Value::DELIVERED) {
                return {false, "Cannot cancel delivered order"};
            }
            
            // Cancel
            OrderStatus cancelledStatus(OrderStatus::Value::CANCELLED);
            if (!repository_->updateStatus(dto.orderId, cancelledStatus)) {
                LOG_ERROR("[CancelOrderUseCase] Failed to cancel order");
                return {false, "Failed to cancel order"};
            }
            
            LOG_INFO("[CancelOrderUseCase] Order cancelled: " + dto.orderId + 
                     (dto.reason.empty() ? "" : " - Reason: " + dto.reason));
            return {true, "Order cancelled successfully"};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[CancelOrderUseCase] Error: " + std::string(e.what()));
            return {false, std::string("Error: ") + e.what()};
        }
    }
    
private:
    std::shared_ptr<OrderRepository> repository_;
};

} // namespace Order
} // namespace Domain

