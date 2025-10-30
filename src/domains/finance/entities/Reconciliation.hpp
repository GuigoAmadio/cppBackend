#pragma once
#include <string>
#include <optional>
#include "../value_objects/ReconciliationStatus.hpp"

namespace Finance {

class Reconciliation {
public:
    Reconciliation(
        const std::string& id,
        const std::string& tenantId,
        const std::string& accountId,
        const std::string& statementDate,
        double statementBalance,
        double systemBalance,
        double difference,
        const ReconciliationStatus& status,
        int matchedTransactions,
        int unmatchedTransactions,
        const std::string& createdBy
    ) : id_(id),
        tenantId_(tenantId),
        accountId_(accountId),
        statementDate_(statementDate),
        statementBalance_(statementBalance),
        systemBalance_(systemBalance),
        difference_(difference),
        status_(status),
        matchedTransactions_(matchedTransactions),
        unmatchedTransactions_(unmatchedTransactions),
        createdBy_(createdBy) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getWorkspaceId() const { return workspaceId_; }
    const std::string& getAccountId() const { return accountId_; }
    const std::string& getStatementDate() const { return statementDate_; }
    double getStatementBalance() const { return statementBalance_; }
    double getSystemBalance() const { return systemBalance_; }
    double getDifference() const { return difference_; }
    const ReconciliationStatus& getStatus() const { return status_; }
    int getMatchedTransactions() const { return matchedTransactions_; }
    int getUnmatchedTransactions() const { return unmatchedTransactions_; }
    const std::optional<std::string>& getNotes() const { return notes_; }
    const std::optional<std::string>& getReconciledBy() const { return reconciledBy_; }
    const std::optional<std::string>& getReconciledAt() const { return reconciledAt_; }
    const std::string& getCreatedBy() const { return createdBy_; }

    // Setters
    void setWorkspaceId(const std::string& workspaceId) { workspaceId_ = workspaceId; }
    void setNotes(const std::string& notes) { notes_ = notes; }

    // Business logic
    bool isBalanced() const { return difference_ == 0.0; }
    bool hasUnmatched() const { return unmatchedTransactions_ > 0; }

    void matchTransaction() {
        matchedTransactions_++;
        if (unmatchedTransactions_ > 0) {
            unmatchedTransactions_--;
        }
        updateStatus();
    }

    void unmatchTransaction() {
        if (matchedTransactions_ > 0) {
            matchedTransactions_--;
        }
        unmatchedTransactions_++;
        updateStatus();
    }

    void complete(const std::string& reconciledBy, const std::string& reconciledAt) {
        if (hasUnmatched()) {
            throw std::runtime_error("Cannot complete reconciliation with unmatched transactions");
        }
        status_ = ReconciliationStatus(ReconciliationStatus::Type::COMPLETED);
        reconciledBy_ = reconciledBy;
        reconciledAt_ = reconciledAt;
    }

private:
    void updateStatus() {
        if (status_.isCompleted()) return;
        
        if (hasUnmatched()) {
            status_ = ReconciliationStatus(ReconciliationStatus::Type::UNMATCHED);
        } else if (matchedTransactions_ > 0) {
            status_ = ReconciliationStatus(ReconciliationStatus::Type::MATCHED);
        } else {
            status_ = ReconciliationStatus(ReconciliationStatus::Type::PENDING);
        }
    }

    std::string id_;
    std::string tenantId_;
    std::string workspaceId_;
    std::string accountId_;
    std::string statementDate_;
    double statementBalance_;
    double systemBalance_;
    double difference_;
    ReconciliationStatus status_;
    int matchedTransactions_;
    int unmatchedTransactions_;
    std::optional<std::string> notes_;
    std::optional<std::string> reconciledBy_;
    std::optional<std::string> reconciledAt_;
    std::string createdBy_;
};

} // namespace Finance

