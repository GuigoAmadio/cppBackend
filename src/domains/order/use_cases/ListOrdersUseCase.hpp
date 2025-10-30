#pragma once
#include <memory>
#include <vector>
#include "../repositories/OrderRepository.hpp"

namespace Domain {
namespace Order {

struct ListOrdersDto {
    std::string tenantId;
    int limit = 50;
    int offset = 0;
};

struct ListOrdersResult {
    bool success;
    std::string message;
    std::vector<Order> orders;
    int total;
};

class ListOrdersUseCase {
public:
    explicit ListOrdersUseCase(std::shared_ptr<OrderRepository> repository)
        : repository_(repository) {}
    
    ListOrdersResult execute(const ListOrdersDto& dto) {
        auto orders = repository_->listByTenant(dto.tenantId, dto.limit, dto.offset);
        return {true, "Orders retrieved", orders, static_cast<int>(orders.size())};
    }
    
private:
    std::shared_ptr<OrderRepository> repository_;
};

} // namespace Order
} // namespace Domain

