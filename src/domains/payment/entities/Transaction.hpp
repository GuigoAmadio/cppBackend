#pragma once

#include <string>
#include <optional>
#include "../value_objects/TransactionType.hpp"

namespace Domain::Payment {

/**
 * @brief Entidade Transaction
 * 
 * Representa uma transação de pagamento (captura, reembolso, cancelamento, etc)
 */
class Transaction {
public:
    Transaction(
        const std::string& id,
        const std::string& paymentId,
        const std::string& tenantId,
        TransactionType type,
        const std::string& status,
        double amount,
        const std::string& currency
    ) : id_(id),
        paymentId_(paymentId),
        tenantId_(tenantId),
        type_(type),
        status_(status),
        amount_(amount),
        currency_(currency)
    {
        if (amount_ <= 0) {
            throw std::invalid_argument("Transaction amount must be greater than zero");
        }
        
        if (status_ != "pending" && status_ != "completed" && status_ != "failed") {
            throw std::invalid_argument("Invalid transaction status: " + status_);
        }
    }

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getPaymentId() const { return paymentId_; }
    const std::string& getTenantId() const { return tenantId_; }
    const TransactionType& getType() const { return type_; }
    const std::string& getStatus() const { return status_; }
    double getAmount() const { return amount_; }
    const std::string& getCurrency() const { return currency_; }
    
    const std::optional<std::string>& getProviderTransactionId() const { return providerTransactionId_; }
    const std::optional<std::string>& getProviderResponse() const { return providerResponse_; }
    const std::optional<std::string>& getReason() const { return reason_; }
    const std::optional<std::string>& getNotes() const { return notes_; }
    const std::optional<std::string>& getCreatedBy() const { return createdBy_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }

    // Setters
    void setStatus(const std::string& status) {
        if (status != "pending" && status != "completed" && status != "failed") {
            throw std::invalid_argument("Invalid transaction status: " + status);
        }
        status_ = status;
    }
    
    void setProviderTransactionId(const std::string& id) { providerTransactionId_ = id; }
    void setProviderResponse(const std::string& response) { providerResponse_ = response; }
    void setReason(const std::string& reason) { reason_ = reason; }
    void setNotes(const std::string& notes) { notes_ = notes; }
    void setCreatedBy(const std::string& createdBy) { createdBy_ = createdBy; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }

    // Business logic
    void markAsCompleted() {
        status_ = "completed";
    }

    void markAsFailed(const std::string& reason) {
        status_ = "failed";
        reason_ = reason;
    }

    bool isPending() const { return status_ == "pending"; }
    bool isCompleted() const { return status_ == "completed"; }
    bool isFailed() const { return status_ == "failed"; }

private:
    std::string id_;
    std::string paymentId_;
    std::string tenantId_;
    TransactionType type_;
    std::string status_;  // "pending", "completed", "failed"
    double amount_;
    std::string currency_;

    // Optional fields
    std::optional<std::string> providerTransactionId_;
    std::optional<std::string> providerResponse_;
    std::optional<std::string> reason_;
    std::optional<std::string> notes_;
    std::optional<std::string> createdBy_;
    std::optional<std::string> createdAt_;
};

} // namespace Domain::Payment

