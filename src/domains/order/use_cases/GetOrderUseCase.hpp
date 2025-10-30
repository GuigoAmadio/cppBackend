#pragma once
#include <memory>
#include <optional>
#include "../repositories/OrderRepository.hpp"

namespace Domain {
namespace Order {

struct GetOrderDto {
    std::string orderId;
    std::string tenantId;
};

struct GetOrderResult {
    bool success;
    std::string message;
    std::optional<Order> order;
};

class GetOrderUseCase {
public:
    explicit GetOrderUseCase(std::shared_ptr<OrderRepository> repository)
        : repository_(repository) {}
    
    GetOrderResult execute(const GetOrderDto& dto) {
        auto order = repository_->findById(dto.orderId);
        
        if (!order.has_value()) {
            return {false, "Order not found", std::nullopt};
        }
        
        if (!dto.tenantId.empty() && order->getTenantId() != dto.tenantId) {
            return {false, "Access denied", std::nullopt};
        }
        
        return {true, "Order found", order};
    }
    
private:
    std::shared_ptr<OrderRepository> repository_;
};

} // namespace Order
} // namespace Domain

