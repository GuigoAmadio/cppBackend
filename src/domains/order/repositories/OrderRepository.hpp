#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "../entities/Order.hpp"
#include "../../../core/database/ConnectionPool.hpp"

namespace Domain {
namespace Order {

class OrderRepository {
public:
    virtual ~OrderRepository() = default;
    
    virtual bool save(const Order& order) = 0;
    virtual bool saveItems(const std::vector<OrderItem>& items) = 0;
    virtual std::optional<Order> findById(const std::string& id) = 0;
    virtual std::vector<Order> listByTenant(const std::string& tenantId, int limit = 50, int offset = 0) = 0;
    virtual std::vector<Order> listByCustomer(const std::string& customerId, int limit = 50) = 0;
    virtual bool updateStatus(const std::string& id, const OrderStatus& status) = 0;
    virtual bool updatePaymentStatus(const std::string& id, const PaymentStatus& status) = 0;
};

// PostgreSQL Implementation
class OrderRepositoryImpl : public OrderRepository {
public:
    explicit OrderRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);
    
    bool save(const Order& order) override;
    bool saveItems(const std::vector<OrderItem>& items) override;
    std::optional<Order> findById(const std::string& id) override;
    std::vector<Order> listByTenant(const std::string& tenantId, int limit, int offset) override;
    std::vector<Order> listByCustomer(const std::string& customerId, int limit) override;
    bool updateStatus(const std::string& id, const OrderStatus& status) override;
    bool updatePaymentStatus(const std::string& id, const PaymentStatus& status) override;
    
private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    std::vector<OrderItem> findItemsByOrderId(const std::string& orderId);
};

// Factory
inline std::shared_ptr<OrderRepository> createOrderRepository(
    std::shared_ptr<Core::Database::ConnectionPool> pool
) {
    return std::make_shared<OrderRepositoryImpl>(pool);
}

} // namespace Order
} // namespace Domain

