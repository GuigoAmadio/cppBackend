#pragma once

#include <string>
#include <optional>
#include "../value_objects/SubscriptionStatus.hpp"

namespace Domain::Subscription {

class Subscription {
public:
    Subscription(
        const std::string& id,
        const std::string& tenantId,
        const std::string& customerId,
        const std::string& planId,
        double billingAmount
    ) : id_(id), tenantId_(tenantId), customerId_(customerId), planId_(planId),
        status_(SubscriptionStatus::Type::TRIAL), billingAmount_(billingAmount),
        currency_("BRL"), billingCyclesCompleted_(0), failedBillingAttempts_(0) {}

    // Getters
    std::string getId() const { return id_; }
    std::string getTenantId() const { return tenantId_; }
    std::string getCustomerId() const { return customerId_; }
    std::string getPlanId() const { return planId_; }
    SubscriptionStatus getStatus() const { return status_; }
    double getBillingAmount() const { return billingAmount_; }
    std::string getCurrency() const { return currency_; }
    int getBillingCyclesCompleted() const { return billingCyclesCompleted_; }
    int getFailedBillingAttempts() const { return failedBillingAttempts_; }
    
    std::optional<std::string> getStartedAt() const { return startedAt_; }
    std::optional<std::string> getTrialEndsAt() const { return trialEndsAt_; }
    std::optional<std::string> getCurrentPeriodStart() const { return currentPeriodStart_; }
    std::optional<std::string> getCurrentPeriodEnd() const { return currentPeriodEnd_; }
    std::optional<std::string> getCancelledAt() const { return cancelledAt_; }
    std::optional<std::string> getExpiresAt() const { return expiresAt_; }
    std::optional<std::string> getPausedAt() const { return pausedAt_; }
    std::optional<std::string> getNextBillingDate() const { return nextBillingDate_; }
    std::optional<std::string> getLastBillingDate() const { return lastBillingDate_; }
    std::optional<std::string> getMetadata() const { return metadata_; }
    std::optional<std::string> getCancelReason() const { return cancelReason_; }
    std::optional<std::string> getCreatedBy() const { return createdBy_; }
    std::optional<std::string> getUpdatedBy() const { return updatedBy_; }
    std::optional<std::string> getCreatedAt() const { return createdAt_; }
    std::optional<std::string> getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setStatus(const SubscriptionStatus& status) { status_ = status; }
    void setBillingAmount(double amount) { 
        if (amount < 0) throw std::invalid_argument("Billing amount cannot be negative");
        billingAmount_ = amount; 
    }
    void setCurrency(const std::string& currency) { currency_ = currency; }
    void setBillingCyclesCompleted(int cycles) { billingCyclesCompleted_ = cycles; }
    void setFailedBillingAttempts(int attempts) { failedBillingAttempts_ = attempts; }
    void setStartedAt(const std::string& startedAt) { startedAt_ = startedAt; }
    void setTrialEndsAt(const std::string& trialEndsAt) { trialEndsAt_ = trialEndsAt; }
    void setCurrentPeriodStart(const std::string& start) { currentPeriodStart_ = start; }
    void setCurrentPeriodEnd(const std::string& end) { currentPeriodEnd_ = end; }
    void setCancelledAt(const std::string& cancelledAt) { cancelledAt_ = cancelledAt; }
    void setExpiresAt(const std::string& expiresAt) { expiresAt_ = expiresAt; }
    void setPausedAt(const std::string& pausedAt) { pausedAt_ = pausedAt; }
    void setNextBillingDate(const std::string& date) { nextBillingDate_ = date; }
    void setLastBillingDate(const std::string& date) { lastBillingDate_ = date; }
    void setMetadata(const std::string& metadata) { metadata_ = metadata; }
    void setCancelReason(const std::string& reason) { cancelReason_ = reason; }
    void setCreatedBy(const std::string& createdBy) { createdBy_ = createdBy; }
    void setUpdatedBy(const std::string& updatedBy) { updatedBy_ = updatedBy; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }

    // Business logic
    void activate() { status_ = SubscriptionStatus(SubscriptionStatus::Type::ACTIVE); }
    void cancel(const std::string& reason) { 
        status_ = SubscriptionStatus(SubscriptionStatus::Type::CANCELLED);
        cancelReason_ = reason;
    }
    void pause() { status_ = SubscriptionStatus(SubscriptionStatus::Type::PAUSED); }
    void markAsPastDue() { status_ = SubscriptionStatus(SubscriptionStatus::Type::PAST_DUE); }
    void expire() { status_ = SubscriptionStatus(SubscriptionStatus::Type::EXPIRED); }
    
    void incrementBillingCycle() { billingCyclesCompleted_++; }
    void incrementFailedAttempts() { failedBillingAttempts_++; }
    void resetFailedAttempts() { failedBillingAttempts_ = 0; }
    
    bool canBeBilled() const { return status_.canBeBilled(); }
    bool canBeUpgraded() const { return status_.canBeUpgraded(); }
    bool canBeCancelled() const { return status_.canBeCancelled(); }

private:
    std::string id_;
    std::string tenantId_;
    std::string customerId_;
    std::string planId_;
    SubscriptionStatus status_;
    double billingAmount_;
    std::string currency_;
    int billingCyclesCompleted_;
    int failedBillingAttempts_;
    
    std::optional<std::string> startedAt_;
    std::optional<std::string> trialEndsAt_;
    std::optional<std::string> currentPeriodStart_;
    std::optional<std::string> currentPeriodEnd_;
    std::optional<std::string> cancelledAt_;
    std::optional<std::string> expiresAt_;
    std::optional<std::string> pausedAt_;
    std::optional<std::string> nextBillingDate_;
    std::optional<std::string> lastBillingDate_;
    std::optional<std::string> metadata_;
    std::optional<std::string> cancelReason_;
    std::optional<std::string> createdBy_;
    std::optional<std::string> updatedBy_;
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
};

} // namespace Domain::Subscription

