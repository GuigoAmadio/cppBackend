#pragma once

#include <string>
#include <optional>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "../value_objects/InvoiceStatus.hpp"
#include "../value_objects/InvoiceType.hpp"
#include "InvoiceItem.hpp"

namespace Domain::Invoicing {

/**
 * @brief Entidade Invoice (Fatura)
 */
class Invoice {
public:
    Invoice(
        const std::string& id,
        const std::string& invoiceNumber,
        const std::string& tenantId,
        const std::string& customerId,
        InvoiceStatus status,
        InvoiceType type
    ) : id_(id),
        invoiceNumber_(invoiceNumber),
        tenantId_(tenantId),
        customerId_(customerId),
        status_(status),
        type_(type),
        subtotal_(0.0),
        tax_(0.0),
        discount_(0.0),
        total_(0.0),
        amountPaid_(0.0),
        amountDue_(0.0),
        currency_("BRL")
    {
        issueDate_ = getCurrentTimestamp();
    }

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getInvoiceNumber() const { return invoiceNumber_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getCustomerId() const { return customerId_; }
    const std::optional<std::string>& getOrderId() const { return orderId_; }
    const std::optional<std::string>& getSubscriptionId() const { return subscriptionId_; }
    double getSubtotal() const { return subtotal_; }
    double getTax() const { return tax_; }
    double getDiscount() const { return discount_; }
    double getTotal() const { return total_; }
    double getAmountPaid() const { return amountPaid_; }
    double getAmountDue() const { return amountDue_; }
    const std::string& getCurrency() const { return currency_; }
    const InvoiceStatus& getStatus() const { return status_; }
    const InvoiceType& getType() const { return type_; }
    const std::string& getIssueDate() const { return issueDate_; }
    const std::optional<std::string>& getDueDate() const { return dueDate_; }
    const std::optional<std::string>& getPaidAt() const { return paidAt_; }
    const std::optional<std::string>& getDescription() const { return description_; }
    const std::optional<std::string>& getNotes() const { return notes_; }
    const std::optional<std::string>& getTerms() const { return terms_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getUpdatedAt() const { return updatedAt_; }
    const std::optional<std::string>& getCreatedBy() const { return createdBy_; }
    const std::optional<std::string>& getUpdatedBy() const { return updatedBy_; }

    // Setters
    void setOrderId(const std::string& orderId) { orderId_ = orderId; }
    void setSubscriptionId(const std::string& subscriptionId) { subscriptionId_ = subscriptionId; }
    void setSubtotal(double subtotal) { subtotal_ = subtotal; recalculateTotal(); }
    void setTax(double tax) { tax_ = tax; recalculateTotal(); }
    void setDiscount(double discount) { discount_ = discount; recalculateTotal(); }
    void setTotal(double total) { total_ = total; recalculateAmountDue(); }
    void setAmountPaid(double amountPaid) { amountPaid_ = amountPaid; recalculateAmountDue(); }
    void setCurrency(const std::string& currency) { currency_ = currency; }
    void setStatus(InvoiceStatus status) { status_ = status; }
    void setIssueDate(const std::string& issueDate) { issueDate_ = issueDate; }
    void setDueDate(const std::string& dueDate) { dueDate_ = dueDate; }
    void setPaidAt(const std::string& paidAt) { paidAt_ = paidAt; }
    void setDescription(const std::string& description) { description_ = description; }
    void setNotes(const std::string& notes) { notes_ = notes; }
    void setTerms(const std::string& terms) { terms_ = terms; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }
    void setCreatedBy(const std::string& createdBy) { createdBy_ = createdBy; }
    void setUpdatedBy(const std::string& updatedBy) { updatedBy_ = updatedBy; }

    // Business logic
    void markAsSent() {
        if (!status_.canBeSent()) {
            throw std::runtime_error("Invoice cannot be sent in current state");
        }
        InvoiceStatus newStatus(InvoiceStatus::Type::SENT);
        status_ = newStatus;
    }

    void markAsPaid() {
        if (!status_.canBePaid()) {
            throw std::runtime_error("Invoice cannot be marked as paid in current state");
        }
        InvoiceStatus newStatus(InvoiceStatus::Type::PAID);
        status_ = newStatus;
        amountPaid_ = total_;
        amountDue_ = 0.0;
        paidAt_ = getCurrentTimestamp();
    }

    void markAsPartiallyPaid(double amount) {
        if (!status_.canBePaid()) {
            throw std::runtime_error("Invoice cannot be marked as paid in current state");
        }
        if (amount <= 0 || amount >= total_) {
            throw std::invalid_argument("Invalid partial payment amount");
        }
        InvoiceStatus newStatus(InvoiceStatus::Type::PARTIAL);
        status_ = newStatus;
        amountPaid_ = amount;
        recalculateAmountDue();
    }

    void markAsCancelled() {
        if (!status_.canBeCancelled()) {
            throw std::runtime_error("Invoice cannot be cancelled in current state");
        }
        InvoiceStatus newStatus(InvoiceStatus::Type::CANCELLED);
        status_ = newStatus;
    }

    void markAsVoid() {
        InvoiceStatus newStatus(InvoiceStatus::Type::VOIDED);
        status_ = newStatus;
    }

    void applyPayment(double amount) {
        if (amount <= 0) {
            throw std::invalid_argument("Payment amount must be greater than zero");
        }
        
        amountPaid_ += amount;
        recalculateAmountDue();
        
        if (amountDue_ <= 0.01) {  // Tolerance for floating point
            markAsPaid();
        } else if (amountPaid_ > 0) {
            InvoiceStatus newStatus(InvoiceStatus::Type::PARTIAL);
            status_ = newStatus;
        }
    }

private:
    std::string id_;
    std::string invoiceNumber_;
    std::string tenantId_;
    std::string customerId_;
    std::optional<std::string> orderId_;
    std::optional<std::string> subscriptionId_;
    
    double subtotal_;
    double tax_;
    double discount_;
    double total_;
    double amountPaid_;
    double amountDue_;
    std::string currency_;
    
    InvoiceStatus status_;
    InvoiceType type_;
    
    std::string issueDate_;
    std::optional<std::string> dueDate_;
    std::optional<std::string> paidAt_;
    
    std::optional<std::string> description_;
    std::optional<std::string> notes_;
    std::optional<std::string> terms_;
    
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
    std::optional<std::string> createdBy_;
    std::optional<std::string> updatedBy_;

    void recalculateTotal() {
        total_ = subtotal_ + tax_ - discount_;
        recalculateAmountDue();
    }

    void recalculateAmountDue() {
        amountDue_ = total_ - amountPaid_;
        if (amountDue_ < 0) amountDue_ = 0;
    }

    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

} // namespace Domain::Invoicing

