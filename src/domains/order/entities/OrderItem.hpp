#pragma once
#include <string>
#include <optional>

namespace Domain {
namespace Order {

class OrderItem {
public:
    OrderItem(
        const std::string& id,
        const std::string& orderId,
        const std::string& productId,
        const std::string& productName,
        double quantity,
        double unitPrice
    ) : id_(id), orderId_(orderId), productId_(productId), productName_(productName),
        quantity_(quantity), unitPrice_(unitPrice), discount_(0), tax_(0) {
        calculateTotals();
    }
    
    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getOrderId() const { return orderId_; }
    const std::string& getProductId() const { return productId_; }
    const std::string& getProductName() const { return productName_; }
    std::optional<std::string> getProductCode() const { return productCode_; }
    double getQuantity() const { return quantity_; }
    double getUnitPrice() const { return unitPrice_; }
    double getDiscount() const { return discount_; }
    double getTax() const { return tax_; }
    double getSubtotal() const { return subtotal_; }
    double getTotal() const { return total_; }
    std::optional<std::string> getNotes() const { return notes_; }
    std::optional<std::string> getCreatedAt() const { return createdAt_; }
    
    // Setters
    void setProductCode(const std::string& code) { productCode_ = code; }
    void setQuantity(double qty) { quantity_ = qty; calculateTotals(); }
    void setUnitPrice(double price) { unitPrice_ = price; calculateTotals(); }
    void setDiscount(double disc) { discount_ = disc; calculateTotals(); }
    void setTax(double t) { tax_ = t; calculateTotals(); }
    void setNotes(const std::string& notes) { notes_ = notes; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    
private:
    void calculateTotals() {
        subtotal_ = quantity_ * unitPrice_;
        total_ = subtotal_ - discount_ + tax_;
    }
    
    std::string id_;
    std::string orderId_;
    std::string productId_;
    std::string productName_;
    std::optional<std::string> productCode_;
    double quantity_;
    double unitPrice_;
    double discount_;
    double tax_;
    double subtotal_;
    double total_;
    std::optional<std::string> notes_;
    std::optional<std::string> createdAt_;
};

} // namespace Order
} // namespace Domain

