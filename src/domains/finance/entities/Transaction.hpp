#pragma once
#include <string>
#include <optional>
#include "../value_objects/TransactionType.hpp"
#include "../value_objects/TransactionStatus.hpp"

namespace Finance {

class Transaction {
public:
    Transaction(
        const std::string& id,
        const std::string& tenantId,
        const std::string& accountId,
        const TransactionType& type,
        double amount,
        const std::string& currency,
        const TransactionStatus& status,
        const std::string& transactionDate,
        const std::string& createdBy
    ) : id_(id),
        tenantId_(tenantId),
        accountId_(accountId),
        type_(type),
        amount_(amount),
        currency_(currency),
        status_(status),
        transactionDate_(transactionDate),
        createdBy_(createdBy) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getWorkspaceId() const { return workspaceId_; }
    const std::string& getAccountId() const { return accountId_; }
    const std::optional<std::string>& getCategoryId() const { return categoryId_; }
    const TransactionType& getType() const { return type_; }
    double getAmount() const { return amount_; }
    const std::string& getCurrency() const { return currency_; }
    const TransactionStatus& getStatus() const { return status_; }
    const std::optional<std::string>& getDescription() const { return description_; }
    const std::string& getTransactionDate() const { return transactionDate_; }
    const std::optional<std::string>& getReferenceId() const { return referenceId_; }
    const std::optional<std::string>& getReferenceType() const { return referenceType_; }
    const std::optional<std::string>& getFromAccountId() const { return fromAccountId_; }
    const std::optional<std::string>& getToAccountId() const { return toAccountId_; }
    const std::optional<std::string>& getReconciliationId() const { return reconciliationId_; }
    const std::optional<std::string>& getReconciledAt() const { return reconciledAt_; }
    const std::string& getCreatedBy() const { return createdBy_; }

    // Setters
    void setWorkspaceId(const std::string& workspaceId) { workspaceId_ = workspaceId; }
    void setCategoryId(const std::string& categoryId) { categoryId_ = categoryId; }
    void setDescription(const std::string& description) { description_ = description; }
    void setReferenceId(const std::string& refId) { referenceId_ = refId; }
    void setReferenceType(const std::string& refType) { referenceType_ = refType; }
    void setFromAccountId(const std::string& fromAccountId) { fromAccountId_ = fromAccountId; }
    void setToAccountId(const std::string& toAccountId) { toAccountId_ = toAccountId; }

    // Business logic
    bool isTransfer() const { return type_.isTransfer(); }

    void complete() {
        if (status_.isCompleted() || status_.isCancelled()) {
            throw std::runtime_error("Transaction already completed or cancelled");
        }
        status_ = TransactionStatus(TransactionStatus::Type::COMPLETED);
    }

    void cancel() {
        if (status_.isCancelled()) {
            throw std::runtime_error("Transaction already cancelled");
        }
        if (status_.isReconciled()) {
            throw std::runtime_error("Cannot cancel a reconciled transaction");
        }
        status_ = TransactionStatus(TransactionStatus::Type::CANCELLED);
    }

    void reconcile(const std::string& reconciliationId, const std::string& reconciledAt) {
        if (!status_.isCompleted()) {
            throw std::runtime_error("Only completed transactions can be reconciled");
        }
        status_ = TransactionStatus(TransactionStatus::Type::RECONCILED);
        reconciliationId_ = reconciliationId;
        reconciledAt_ = reconciledAt;
    }

private:
    std::string id_;
    std::string tenantId_;
    std::string workspaceId_;
    std::string accountId_;
    std::optional<std::string> categoryId_;
    TransactionType type_;
    double amount_;
    std::string currency_;
    TransactionStatus status_;
    std::optional<std::string> description_;
    std::string transactionDate_;
    std::optional<std::string> referenceId_;
    std::optional<std::string> referenceType_;
    std::optional<std::string> fromAccountId_;
    std::optional<std::string> toAccountId_;
    std::optional<std::string> reconciliationId_;
    std::optional<std::string> reconciledAt_;
    std::string createdBy_;
};

} // namespace Finance

