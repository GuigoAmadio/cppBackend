#pragma once
#include <string>
#include <optional>
#include <vector>
#include "../value_objects/OrderStatus.hpp"
#include "../value_objects/PaymentMethod.hpp"
#include "../value_objects/PaymentStatus.hpp"
#include "OrderItem.hpp"

namespace Domain {
namespace Order {

class Order {
public:
    Order(
        const std::string& id,
        const std::string& tenantId,
        const std::string& customerId,
        const std::string& userId,
        const std::string& orderNumber
    ) : id_(id), tenantId_(tenantId), customerId_(customerId), userId_(userId),
        orderNumber_(orderNumber), status_(OrderStatus::Value::PENDING),
        paymentStatus_(PaymentStatus::Value::PENDING),
        subtotal_(0), discount_(0), tax_(0), shippingCost_(0), total_(0) {}
    
    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getCustomerId() const { return customerId_; }
    const std::string& getUserId() const { return userId_; }
    const std::string& getOrderNumber() const { return orderNumber_; }
    const OrderStatus& getStatus() const { return status_; }
    const PaymentStatus& getPaymentStatus() const { return paymentStatus_; }
    std::optional<PaymentMethod> getPaymentMethod() const { return paymentMethod_; }
    
    double getSubtotal() const { return subtotal_; }
    double getDiscount() const { return discount_; }
    double getTax() const { return tax_; }
    double getShippingCost() const { return shippingCost_; }
    double getTotal() const { return total_; }
    
    std::optional<std::string> getShippingAddress() const { return shippingAddress_; }
    std::optional<std::string> getShippingCity() const { return shippingCity_; }
    std::optional<std::string> getShippingState() const { return shippingState_; }
    std::optional<std::string> getShippingZipCode() const { return shippingZipCode_; }
    std::optional<std::string> getShippingCountry() const { return shippingCountry_; }
    
    std::optional<std::string> getCustomerNotes() const { return customerNotes_; }
    std::optional<std::string> getInternalNotes() const { return internalNotes_; }
    
    std::optional<std::string> getCreatedAt() const { return createdAt_; }
    std::optional<std::string> getUpdatedAt() const { return updatedAt_; }
    std::optional<std::string> getConfirmedAt() const { return confirmedAt_; }
    std::optional<std::string> getShippedAt() const { return shippedAt_; }
    std::optional<std::string> getDeliveredAt() const { return deliveredAt_; }
    std::optional<std::string> getCancelledAt() const { return cancelledAt_; }
    
    const std::vector<OrderItem>& getItems() const { return items_; }
    
    // Setters
    void setStatus(const OrderStatus& status) { status_ = status; }
    void setPaymentStatus(const PaymentStatus& status) { paymentStatus_ = status; }
    void setPaymentMethod(const PaymentMethod& method) { paymentMethod_ = method; }
    
    void setSubtotal(double value) { subtotal_ = value; calculateTotal(); }
    void setDiscount(double value) { discount_ = value; calculateTotal(); }
    void setTax(double value) { tax_ = value; calculateTotal(); }
    void setShippingCost(double value) { shippingCost_ = value; calculateTotal(); }
    
    void setShippingAddress(const std::string& addr) { shippingAddress_ = addr; }
    void setShippingCity(const std::string& city) { shippingCity_ = city; }
    void setShippingState(const std::string& state) { shippingState_ = state; }
    void setShippingZipCode(const std::string& zip) { shippingZipCode_ = zip; }
    void setShippingCountry(const std::string& country) { shippingCountry_ = country; }
    
    void setCustomerNotes(const std::string& notes) { customerNotes_ = notes; }
    void setInternalNotes(const std::string& notes) { internalNotes_ = notes; }
    
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }
    void setConfirmedAt(const std::string& confirmedAt) { confirmedAt_ = confirmedAt; }
    void setShippedAt(const std::string& shippedAt) { shippedAt_ = shippedAt; }
    void setDeliveredAt(const std::string& deliveredAt) { deliveredAt_ = deliveredAt; }
    void setCancelledAt(const std::string& cancelledAt) { cancelledAt_ = cancelledAt; }
    
    // Item management
    void addItem(const OrderItem& item) {
        items_.push_back(item);
        recalculateFromItems();
    }
    
    void setItems(const std::vector<OrderItem>& items) {
        items_ = items;
        recalculateFromItems();
    }
    
private:
    void calculateTotal() {
        total_ = subtotal_ - discount_ + tax_ + shippingCost_;
    }
    
    void recalculateFromItems() {
        subtotal_ = 0;
        for (const auto& item : items_) {
            subtotal_ += item.getSubtotal();
        }
        calculateTotal();
    }
    
    std::string id_;
    std::string tenantId_;
    std::string customerId_;
    std::string userId_;
    std::string orderNumber_;
    
    OrderStatus status_;
    PaymentStatus paymentStatus_;
    std::optional<PaymentMethod> paymentMethod_;
    
    double subtotal_;
    double discount_;
    double tax_;
    double shippingCost_;
    double total_;
    
    std::optional<std::string> shippingAddress_;
    std::optional<std::string> shippingCity_;
    std::optional<std::string> shippingState_;
    std::optional<std::string> shippingZipCode_;
    std::optional<std::string> shippingCountry_;
    
    std::optional<std::string> customerNotes_;
    std::optional<std::string> internalNotes_;
    
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
    std::optional<std::string> confirmedAt_;
    std::optional<std::string> shippedAt_;
    std::optional<std::string> deliveredAt_;
    std::optional<std::string> cancelledAt_;
    
    std::vector<OrderItem> items_;
};

} // namespace Order
} // namespace Domain

