#pragma once

#include <string>
#include <optional>
#include "../value_objects/BillingStatus.hpp"

namespace Domain::Subscription {

class BillingHistory {
public:
    BillingHistory(
        const std::string& id,
        const std::string& subscriptionId,
        const std::string& tenantId,
        double amount
    ) : id_(id), subscriptionId_(subscriptionId), tenantId_(tenantId),
        amount_(amount), currency_("BRL"), status_(BillingStatus::Type::PENDING) {}

    // Getters
    std::string getId() const { return id_; }
    std::string getSubscriptionId() const { return subscriptionId_; }
    std::string getTenantId() const { return tenantId_; }
    double getAmount() const { return amount_; }
    std::string getCurrency() const { return currency_; }
    BillingStatus getStatus() const { return status_; }
    
    std::optional<std::string> getBillingDate() const { return billingDate_; }
    std::optional<std::string> getPeriodStart() const { return periodStart_; }
    std::optional<std::string> getPeriodEnd() const { return periodEnd_; }
    std::optional<std::string> getPaymentId() const { return paymentId_; }
    std::optional<std::string> getPaymentMethod() const { return paymentMethod_; }
    std::optional<std::string> getInvoiceNumber() const { return invoiceNumber_; }
    std::optional<std::string> getInvoiceUrl() const { return invoiceUrl_; }
    std::optional<std::string> getFailureReason() const { return failureReason_; }
    std::optional<std::string> getMetadata() const { return metadata_; }
    std::optional<std::string> getCreatedAt() const { return createdAt_; }
    std::optional<std::string> getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setAmount(double amount) { 
        if (amount < 0) throw std::invalid_argument("Amount cannot be negative");
        amount_ = amount; 
    }
    void setCurrency(const std::string& currency) { currency_ = currency; }
    void setStatus(const BillingStatus& status) { status_ = status; }
    void setBillingDate(const std::string& date) { billingDate_ = date; }
    void setPeriodStart(const std::string& start) { periodStart_ = start; }
    void setPeriodEnd(const std::string& end) { periodEnd_ = end; }
    void setPaymentId(const std::string& paymentId) { paymentId_ = paymentId; }
    void setPaymentMethod(const std::string& method) { paymentMethod_ = method; }
    void setInvoiceNumber(const std::string& number) { invoiceNumber_ = number; }
    void setInvoiceUrl(const std::string& url) { invoiceUrl_ = url; }
    void setFailureReason(const std::string& reason) { failureReason_ = reason; }
    void setMetadata(const std::string& metadata) { metadata_ = metadata; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }

    // Business logic
    void markAsPaid(const std::string& paymentId, const std::string& paymentMethod) {
        status_ = BillingStatus(BillingStatus::Type::PAID);
        paymentId_ = paymentId;
        paymentMethod_ = paymentMethod;
    }

    void markAsFailed(const std::string& reason) {
        status_ = BillingStatus(BillingStatus::Type::FAILED);
        failureReason_ = reason;
    }

    void markAsRefunded() {
        status_ = BillingStatus(BillingStatus::Type::REFUNDED);
    }

private:
    std::string id_;
    std::string subscriptionId_;
    std::string tenantId_;
    double amount_;
    std::string currency_;
    BillingStatus status_;
    
    std::optional<std::string> billingDate_;
    std::optional<std::string> periodStart_;
    std::optional<std::string> periodEnd_;
    std::optional<std::string> paymentId_;
    std::optional<std::string> paymentMethod_;
    std::optional<std::string> invoiceNumber_;
    std::optional<std::string> invoiceUrl_;
    std::optional<std::string> failureReason_;
    std::optional<std::string> metadata_;
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
};

} // namespace Domain::Subscription

