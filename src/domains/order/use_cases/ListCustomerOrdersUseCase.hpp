#pragma once
#include <memory>
#include <vector>
#include "../repositories/OrderRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Order {

struct ListCustomerOrdersDto {
    std::string customerId;
    std::string tenantId;
    int limit = 50;
};

struct ListCustomerOrdersResult {
    bool success;
    std::string message;
    std::vector<Order> orders;
    int total;
};

class ListCustomerOrdersUseCase {
public:
    explicit ListCustomerOrdersUseCase(std::shared_ptr<OrderRepository> repository)
        : repository_(repository) {}
    
    ListCustomerOrdersResult execute(const ListCustomerOrdersDto& dto) {
        try {
            LOG_DEBUG("[ListCustomerOrdersUseCase] Listing orders for customer: " + dto.customerId);
            
            auto orders = repository_->listByCustomer(dto.customerId, dto.limit);
            
            // Filter by tenant if provided
            if (!dto.tenantId.empty()) {
                std::vector<Order> filteredOrders;
                for (const auto& order : orders) {
                    if (order.getTenantId() == dto.tenantId) {
                        filteredOrders.push_back(order);
                    }
                }
                orders = filteredOrders;
            }
            
            LOG_INFO("[ListCustomerOrdersUseCase] Found " + std::to_string(orders.size()) + " orders");
            return {true, "Orders retrieved", orders, static_cast<int>(orders.size())};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[ListCustomerOrdersUseCase] Error: " + std::string(e.what()));
            return {false, std::string("Error: ") + e.what(), {}, 0};
        }
    }
    
private:
    std::shared_ptr<OrderRepository> repository_;
};

} // namespace Order
} // namespace Domain

