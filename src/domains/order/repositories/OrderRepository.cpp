#include "OrderRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>

namespace Domain {
namespace Order {

OrderRepositoryImpl::OrderRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {}

bool OrderRepositoryImpl::save(const Order& order) {
    LOG_DEBUG("[OrderRepository] save() - order: " + order.getId());
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        INSERT INTO orders (
            id, tenant_id, customer_id, user_id, order_number,
            status, subtotal, discount, tax, shipping_cost, total,
            payment_method, payment_status,
            shipping_address, shipping_city, shipping_state, shipping_zip_code, shipping_country,
            customer_notes, internal_notes,
            created_at, updated_at
        ) VALUES (
            $1, $2, $3, $4, $5,
            $6, $7, $8, $9, $10, $11,
            NULLIF($12,''), $13,
            NULLIF($14,''), NULLIF($15,''), NULLIF($16,''), NULLIF($17,''), NULLIF($18,''),
            NULLIF($19,''), NULLIF($20,''),
            CURRENT_TIMESTAMP, CURRENT_TIMESTAMP
        )
        ON CONFLICT (id) DO UPDATE SET
            status = EXCLUDED.status,
            payment_status = EXCLUDED.payment_status,
            payment_method = EXCLUDED.payment_method,
            subtotal = EXCLUDED.subtotal,
            discount = EXCLUDED.discount,
            tax = EXCLUDED.tax,
            shipping_cost = EXCLUDED.shipping_cost,
            total = EXCLUDED.total,
            shipping_address = EXCLUDED.shipping_address,
            shipping_city = EXCLUDED.shipping_city,
            shipping_state = EXCLUDED.shipping_state,
            shipping_zip_code = EXCLUDED.shipping_zip_code,
            shipping_country = EXCLUDED.shipping_country,
            customer_notes = EXCLUDED.customer_notes,
            internal_notes = EXCLUDED.internal_notes,
            updated_at = CURRENT_TIMESTAMP
    )";
    
    std::vector<std::string> params = {
        order.getId(),
        order.getTenantId(),
        order.getCustomerId(),
        order.getUserId(),
        order.getOrderNumber(),
        order.getStatus().toString(),
        std::to_string(order.getSubtotal()),
        std::to_string(order.getDiscount()),
        std::to_string(order.getTax()),
        std::to_string(order.getShippingCost()),
        std::to_string(order.getTotal()),
        order.getPaymentMethod().has_value() ? order.getPaymentMethod()->toString() : "",
        order.getPaymentStatus().toString(),
        order.getShippingAddress().value_or(""),
        order.getShippingCity().value_or(""),
        order.getShippingState().value_or(""),
        order.getShippingZipCode().value_or(""),
        order.getShippingCountry().value_or("BR"),
        order.getCustomerNotes().value_or(""),
        order.getInternalNotes().value_or("")
    };
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        LOG_ERROR("[OrderRepository] Failed to save order: " + result.getError());
        return false;
    }
    
    LOG_INFO("[OrderRepository] Order saved: " + order.getId());
    return true;
}

bool OrderRepositoryImpl::saveItems(const std::vector<OrderItem>& items) {
    if (items.empty()) return true;
    
    LOG_DEBUG("[OrderRepository] saveItems() - count: " + std::to_string(items.size()));
    
    auto conn = pool_->acquire();
    
    for (const auto& item : items) {
        std::string query = R"(
            INSERT INTO order_items (
                id, order_id, product_id, product_name, product_code,
                quantity, unit_price, discount, tax, subtotal, total,
                notes, created_at
            ) VALUES (
                $1, $2, $3, $4, NULLIF($5,''),
                $6, $7, $8, $9, $10, $11,
                NULLIF($12,''), CURRENT_TIMESTAMP
            )
        )";
        
        std::vector<std::string> params = {
            item.getId(),
            item.getOrderId(),
            item.getProductId(),
            item.getProductName(),
            item.getProductCode().value_or(""),
            std::to_string(item.getQuantity()),
            std::to_string(item.getUnitPrice()),
            std::to_string(item.getDiscount()),
            std::to_string(item.getTax()),
            std::to_string(item.getSubtotal()),
            std::to_string(item.getTotal()),
            item.getNotes().value_or("")
        };
        
        auto result = conn->executeParams(query, params);
        
        if (!result.isSuccess()) {
            LOG_ERROR("[OrderRepository] Failed to save item: " + result.getError());
            return false;
        }
    }
    
    LOG_INFO("[OrderRepository] Items saved: " + std::to_string(items.size()));
    return true;
}

std::optional<Order> OrderRepositoryImpl::findById(const std::string& id) {
    LOG_DEBUG("[OrderRepository] findById() - id: " + id);
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, customer_id, user_id, order_number,
               status, subtotal, discount, tax, shipping_cost, total,
               payment_method, payment_status,
               shipping_address, shipping_city, shipping_state, shipping_zip_code, shipping_country,
               customer_notes, internal_notes,
               created_at, updated_at, confirmed_at, shipped_at, delivered_at, cancelled_at
        FROM orders
        WHERE id = $1
    )";
    
    auto result = conn->executeParams(query, {id});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        return std::nullopt;
    }
    
    Order order(
        result.getValue(0, 0),  // id
        result.getValue(0, 1),  // tenant_id
        result.getValue(0, 2),  // customer_id
        result.getValue(0, 3),  // user_id
        result.getValue(0, 4)   // order_number
    );
    
    order.setStatus(OrderStatus(result.getValue(0, 5)));
    order.setSubtotal(std::stod(result.getValue(0, 6)));
    order.setDiscount(std::stod(result.getValue(0, 7)));
    order.setTax(std::stod(result.getValue(0, 8)));
    order.setShippingCost(std::stod(result.getValue(0, 9)));
    
    if (!result.getValue(0, 11).empty()) {
        order.setPaymentMethod(PaymentMethod(result.getValue(0, 11)));
    }
    
    std::string paymentStatusStr = result.getValue(0, 12);
    LOG_DEBUG("[OrderRepository] findById() - payment_status from DB: [" + paymentStatusStr + "]");
    order.setPaymentStatus(PaymentStatus(paymentStatusStr));
    LOG_DEBUG("[OrderRepository] findById() - payment_status after set: [" + order.getPaymentStatus().toString() + "]");
    
    if (!result.getValue(0, 13).empty()) order.setShippingAddress(result.getValue(0, 13));
    if (!result.getValue(0, 14).empty()) order.setShippingCity(result.getValue(0, 14));
    if (!result.getValue(0, 15).empty()) order.setShippingState(result.getValue(0, 15));
    if (!result.getValue(0, 16).empty()) order.setShippingZipCode(result.getValue(0, 16));
    if (!result.getValue(0, 17).empty()) order.setShippingCountry(result.getValue(0, 17));
    if (!result.getValue(0, 18).empty()) order.setCustomerNotes(result.getValue(0, 18));
    if (!result.getValue(0, 19).empty()) order.setInternalNotes(result.getValue(0, 19));
    
    order.setCreatedAt(result.getValue(0, 20));
    order.setUpdatedAt(result.getValue(0, 21));
    if (!result.getValue(0, 22).empty()) order.setConfirmedAt(result.getValue(0, 22));
    if (!result.getValue(0, 23).empty()) order.setShippedAt(result.getValue(0, 23));
    if (!result.getValue(0, 24).empty()) order.setDeliveredAt(result.getValue(0, 24));
    if (!result.getValue(0, 25).empty()) order.setCancelledAt(result.getValue(0, 25));
    
    // Load items
    auto items = findItemsByOrderId(id);
    order.setItems(items);
    
    return order;
}

std::vector<OrderItem> OrderRepositoryImpl::findItemsByOrderId(const std::string& orderId) {
    std::vector<OrderItem> items;
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, order_id, product_id, product_name, product_code,
               quantity, unit_price, discount, tax, subtotal, total, notes
        FROM order_items
        WHERE order_id = $1
        ORDER BY created_at
    )";
    
    auto result = conn->executeParams(query, {orderId});
    
    if (!result.isSuccess()) return items;
    
    for (int i = 0; i < result.rowCount(); i++) {
        OrderItem item(
            result.getValue(i, 0),  // id
            result.getValue(i, 1),  // order_id
            result.getValue(i, 2),  // product_id
            result.getValue(i, 3),  // product_name
            std::stod(result.getValue(i, 5)),  // quantity
            std::stod(result.getValue(i, 6))   // unit_price
        );
        
        if (!result.getValue(i, 4).empty()) item.setProductCode(result.getValue(i, 4));
        item.setDiscount(std::stod(result.getValue(i, 7)));
        item.setTax(std::stod(result.getValue(i, 8)));
        if (!result.getValue(i, 11).empty()) item.setNotes(result.getValue(i, 11));
        
        items.push_back(item);
    }
    
    return items;
}

std::vector<Order> OrderRepositoryImpl::listByTenant(const std::string& tenantId, int limit, int offset) {
    std::vector<Order> orders;
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id FROM orders
        WHERE tenant_id = $1
        ORDER BY created_at DESC
        LIMIT $2 OFFSET $3
    )";
    
    auto result = conn->executeParams(query, {tenantId, std::to_string(limit), std::to_string(offset)});
    
    if (!result.isSuccess()) return orders;
    
    for (int i = 0; i < result.rowCount(); i++) {
        auto order = findById(result.getValue(i, 0));
        if (order.has_value()) {
            orders.push_back(*order);
        }
    }
    
    return orders;
}

std::vector<Order> OrderRepositoryImpl::listByCustomer(const std::string& customerId, int limit) {
    std::vector<Order> orders;
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id FROM orders
        WHERE customer_id = $1
        ORDER BY created_at DESC
        LIMIT $2
    )";
    
    auto result = conn->executeParams(query, {customerId, std::to_string(limit)});
    
    if (!result.isSuccess()) return orders;
    
    for (int i = 0; i < result.rowCount(); i++) {
        auto order = findById(result.getValue(i, 0));
        if (order.has_value()) {
            orders.push_back(*order);
        }
    }
    
    return orders;
}

bool OrderRepositoryImpl::updateStatus(const std::string& id, const OrderStatus& status) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE orders
        SET status = $1, updated_at = CURRENT_TIMESTAMP
        WHERE id = $2
    )";
    
    auto result = conn->executeParams(query, {status.toString(), id});
    return result.isSuccess();
}

bool OrderRepositoryImpl::updatePaymentStatus(const std::string& id, const PaymentStatus& status) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE orders
        SET payment_status = $1, updated_at = CURRENT_TIMESTAMP
        WHERE id = $2
    )";
    
    auto result = conn->executeParams(query, {status.toString(), id});
    return result.isSuccess();
}

} // namespace Order
} // namespace Domain

