#pragma once

#include <string>
#include <optional>

namespace Domain::Invoicing {

/**
 * @brief Item/linha de uma fatura
 */
class InvoiceItem {
public:
    InvoiceItem(
        const std::string& id,
        const std::string& invoiceId,
        const std::string& tenantId,
        const std::string& description,
        double quantity,
        double unitPrice
    ) : id_(id),
        invoiceId_(invoiceId),
        tenantId_(tenantId),
        description_(description),
        quantity_(quantity),
        unitPrice_(unitPrice),
        discount_(0.0),
        taxRate_(0.0),
        taxAmount_(0.0),
        sortOrder_(0)
    {
        if (quantity_ <= 0) {
            throw std::invalid_argument("Quantity must be greater than zero");
        }
        if (unitPrice_ < 0) {
            throw std::invalid_argument("Unit price cannot be negative");
        }
        calculateTotal();
    }

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getInvoiceId() const { return invoiceId_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::optional<std::string>& getProductId() const { return productId_; }
    const std::string& getDescription() const { return description_; }
    double getQuantity() const { return quantity_; }
    double getUnitPrice() const { return unitPrice_; }
    double getDiscount() const { return discount_; }
    double getTaxRate() const { return taxRate_; }
    double getTaxAmount() const { return taxAmount_; }
    double getTotal() const { return total_; }
    int getSortOrder() const { return sortOrder_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }

    // Setters
    void setProductId(const std::string& productId) { productId_ = productId; }
    void setDescription(const std::string& description) { description_ = description; }
    void setQuantity(double quantity) { 
        if (quantity <= 0) throw std::invalid_argument("Quantity must be greater than zero");
        quantity_ = quantity;
        calculateTotal();
    }
    void setUnitPrice(double unitPrice) { 
        if (unitPrice < 0) throw std::invalid_argument("Unit price cannot be negative");
        unitPrice_ = unitPrice;
        calculateTotal();
    }
    void setDiscount(double discount) { 
        if (discount < 0) throw std::invalid_argument("Discount cannot be negative");
        discount_ = discount;
        calculateTotal();
    }
    void setTaxRate(double taxRate) { 
        if (taxRate < 0) throw std::invalid_argument("Tax rate cannot be negative");
        taxRate_ = taxRate;
        calculateTotal();
    }
    void setSortOrder(int order) { sortOrder_ = order; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }

    // Business logic
    double getSubtotal() const {
        return (quantity_ * unitPrice_) - discount_;
    }

private:
    std::string id_;
    std::string invoiceId_;
    std::string tenantId_;
    std::optional<std::string> productId_;
    std::string description_;
    double quantity_;
    double unitPrice_;
    double discount_;
    double taxRate_;
    double taxAmount_;
    double total_;
    int sortOrder_;
    std::optional<std::string> createdAt_;

    void calculateTotal() {
        double subtotal = getSubtotal();
        taxAmount_ = subtotal * (taxRate_ / 100.0);
        total_ = subtotal + taxAmount_;
    }
};

} // namespace Domain::Invoicing

