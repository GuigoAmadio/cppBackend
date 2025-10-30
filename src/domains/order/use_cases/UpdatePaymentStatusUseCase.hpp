#pragma once
#include <memory>
#include "../repositories/OrderRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Order {

struct UpdatePaymentStatusDto {
    std::string orderId;
    std::string tenantId;
    std::string paymentStatus;  // pending, paid, failed, refunded
};

struct UpdatePaymentStatusResult {
    bool success;
    std::string message;
};

class UpdatePaymentStatusUseCase {
public:
    explicit UpdatePaymentStatusUseCase(std::shared_ptr<OrderRepository> repository)
        : repository_(repository) {}
    
    UpdatePaymentStatusResult execute(const UpdatePaymentStatusDto& dto) {
        try {
            LOG_DEBUG("[UpdatePaymentStatusUseCase] Updating payment for order: " + dto.orderId);
            
            // Validate payment status
            PaymentStatus newStatus;
            try {
                newStatus = PaymentStatus(dto.paymentStatus);
            } catch (const std::exception& e) {
                LOG_WARNING("[UpdatePaymentStatusUseCase] Invalid payment status: " + dto.paymentStatus);
                return {false, "Invalid payment status: " + dto.paymentStatus};
            }
            
            // Get order to validate tenant
            auto orderOpt = repository_->findById(dto.orderId);
            if (!orderOpt.has_value()) {
                LOG_WARNING("[UpdatePaymentStatusUseCase] Order not found: " + dto.orderId);
                return {false, "Order not found"};
            }
            
            if (!dto.tenantId.empty() && orderOpt->getTenantId() != dto.tenantId) {
                LOG_WARNING("[UpdatePaymentStatusUseCase] Access denied for tenant: " + dto.tenantId);
                return {false, "Access denied"};
            }
            
            // Validate status transition
            if (orderOpt->getStatus().getValue() == OrderStatus::Value::CANCELLED) {
                return {false, "Cannot update payment status of cancelled order"};
            }
            
            // Update
            if (!repository_->updatePaymentStatus(dto.orderId, newStatus)) {
                LOG_ERROR("[UpdatePaymentStatusUseCase] Failed to update payment status");
                return {false, "Failed to update payment status"};
            }
            
            LOG_INFO("[UpdatePaymentStatusUseCase] Payment status updated: " + dto.orderId + " -> " + dto.paymentStatus);
            return {true, "Payment status updated successfully"};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[UpdatePaymentStatusUseCase] Error: " + std::string(e.what()));
            return {false, std::string("Error: ") + e.what()};
        }
    }
    
private:
    std::shared_ptr<OrderRepository> repository_;
};

} // namespace Order
} // namespace Domain

