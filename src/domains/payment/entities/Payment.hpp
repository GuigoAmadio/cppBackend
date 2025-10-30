#pragma once

#include <string>
#include <optional>
#include <chrono>
#include "../value_objects/PaymentProvider.hpp"
#include "../value_objects/PaymentStatus.hpp"

namespace Domain::Payment {

/**
 * @brief Entidade Payment
 * 
 * Representa um pagamento realizado por um cliente para uma order ou subscription
 */
class Payment {
public:
    Payment(
        const std::string& id,
        const std::string& customerId,
        const std::string& tenantId,
        double amount,
        const std::string& currency,
        PaymentProvider provider,
        PaymentStatus status
    ) : id_(id),
        customerId_(customerId),
        tenantId_(tenantId),
        amount_(amount),
        currency_(currency),
        provider_(provider),
        status_(status)
    {
        if (amount_ <= 0) {
            throw std::invalid_argument("Amount must be greater than zero");
        }
    }

    // Getters
    const std::string& getId() const { return id_; }
    const std::optional<std::string>& getOrderId() const { return orderId_; }
    const std::optional<std::string>& getSubscriptionId() const { return subscriptionId_; }
    const std::string& getCustomerId() const { return customerId_; }
    const std::string& getTenantId() const { return tenantId_; }
    double getAmount() const { return amount_; }
    const std::string& getCurrency() const { return currency_; }
    const PaymentProvider& getProvider() const { return provider_; }
    const PaymentStatus& getStatus() const { return status_; }
    
    const std::optional<std::string>& getProviderPaymentId() const { return providerPaymentId_; }
    const std::optional<std::string>& getProviderMetadata() const { return providerMetadata_; }
    const std::optional<std::string>& getPaymentMethod() const { return paymentMethod_; }
    const std::optional<std::string>& getPaymentDetails() const { return paymentDetails_; }
    const std::optional<std::string>& getPaidAt() const { return paidAt_; }
    const std::optional<std::string>& getExpiresAt() const { return expiresAt_; }
    const std::optional<std::string>& getCreatedBy() const { return createdBy_; }
    const std::optional<std::string>& getUpdatedBy() const { return updatedBy_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setOrderId(const std::string& orderId) { orderId_ = orderId; }
    void setSubscriptionId(const std::string& subscriptionId) { subscriptionId_ = subscriptionId; }
    void setStatus(PaymentStatus status) { status_ = status; }
    void setProviderPaymentId(const std::string& id) { providerPaymentId_ = id; }
    void setProviderMetadata(const std::string& metadata) { providerMetadata_ = metadata; }
    void setPaymentMethod(const std::string& method) { paymentMethod_ = method; }
    void setPaymentDetails(const std::string& details) { paymentDetails_ = details; }
    void setPaidAt(const std::string& paidAt) { paidAt_ = paidAt; }
    void setExpiresAt(const std::string& expiresAt) { expiresAt_ = expiresAt; }
    void setCreatedBy(const std::string& createdBy) { createdBy_ = createdBy; }
    void setUpdatedBy(const std::string& updatedBy) { updatedBy_ = updatedBy; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }

    // Business logic
    void markAsCompleted() {
        status_ = PaymentStatus(PaymentStatus::Type::COMPLETED);
        paidAt_ = getCurrentTimestamp();
    }

    void markAsFailed() {
        status_ = PaymentStatus(PaymentStatus::Type::FAILED);
    }

    void markAsRefunded() {
        if (!status_.isCompleted()) {
            throw std::runtime_error("Cannot refund a payment that is not completed");
        }
        status_ = PaymentStatus(PaymentStatus::Type::REFUNDED);
    }

    void markAsCancelled() {
        if (status_.isCompleted() || status_.isRefunded()) {
            throw std::runtime_error("Cannot cancel a completed or refunded payment");
        }
        status_ = PaymentStatus(PaymentStatus::Type::CANCELLED);
    }

    bool canBeRefunded() const {
        return status_.isCompleted();
    }

    bool canBeCancelled() const {
        return !status_.isCompleted() && !status_.isRefunded();
    }

private:
    std::string id_;
    std::string customerId_;
    std::string tenantId_;
    double amount_;
    std::string currency_;
    PaymentProvider provider_;
    PaymentStatus status_;

    // Order or Subscription (mutually exclusive, at least one must be set)
    std::optional<std::string> orderId_;
    std::optional<std::string> subscriptionId_;

    // Optional fields
    std::optional<std::string> providerPaymentId_;
    std::optional<std::string> providerMetadata_;
    std::optional<std::string> paymentMethod_;
    std::optional<std::string> paymentDetails_;
    std::optional<std::string> paidAt_;
    std::optional<std::string> expiresAt_;
    std::optional<std::string> createdBy_;
    std::optional<std::string> updatedBy_;
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;

    std::string getCurrentTimestamp() const {
        // Simple ISO 8601 timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));
        return std::string(buffer);
    }
};

} // namespace Domain::Payment

