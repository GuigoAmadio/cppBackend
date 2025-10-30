#pragma once
#include <string>
#include <optional>
#include "../value_objects/BudgetPeriod.hpp"

namespace Finance {

class Budget {
public:
    Budget(
        const std::string& id,
        const std::string& tenantId,
        const std::string& name,
        double amount,
        double spent,
        const BudgetPeriod& period,
        const std::string& startDate,
        const std::string& endDate,
        bool isRecurring,
        int alertPercentage,
        const std::string& status,
        const std::string& createdBy
    ) : id_(id),
        tenantId_(tenantId),
        name_(name),
        amount_(amount),
        spent_(spent),
        period_(period),
        startDate_(startDate),
        endDate_(endDate),
        isRecurring_(isRecurring),
        alertPercentage_(alertPercentage),
        status_(status),
        createdBy_(createdBy) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getWorkspaceId() const { return workspaceId_; }
    const std::string& getName() const { return name_; }
    const std::optional<std::string>& getCategoryId() const { return categoryId_; }
    double getAmount() const { return amount_; }
    double getSpent() const { return spent_; }
    const BudgetPeriod& getPeriod() const { return period_; }
    const std::string& getStartDate() const { return startDate_; }
    const std::string& getEndDate() const { return endDate_; }
    bool isRecurring() const { return isRecurring_; }
    int getAlertPercentage() const { return alertPercentage_; }
    const std::string& getStatus() const { return status_; }
    const std::string& getCreatedBy() const { return createdBy_; }

    // Setters
    void setWorkspaceId(const std::string& workspaceId) { workspaceId_ = workspaceId; }
    void setCategoryId(const std::string& categoryId) { categoryId_ = categoryId; }
    void setName(const std::string& name) {
        if (name.empty()) {
            throw std::invalid_argument("Budget name cannot be empty");
        }
        name_ = name;
    }
    void setAmount(double amount) {
        if (amount <= 0) {
            throw std::invalid_argument("Budget amount must be positive");
        }
        amount_ = amount;
    }
    void setAlertPercentage(int percentage) {
        if (percentage < 0 || percentage > 100) {
            throw std::invalid_argument("Alert percentage must be between 0 and 100");
        }
        alertPercentage_ = percentage;
    }

    // Business logic
    double getRemaining() const { return amount_ - spent_; }
    double getPercentageSpent() const {
        if (amount_ == 0) return 0;
        return (spent_ / amount_) * 100.0;
    }

    bool isExceeded() const { return spent_ > amount_; }
    bool shouldAlert() const { return getPercentageSpent() >= alertPercentage_; }

    void addSpending(double spendingAmount) {
        if (spendingAmount < 0) {
            throw std::invalid_argument("Spending amount cannot be negative");
        }
        spent_ += spendingAmount;
        updateStatus();
    }

    void subtractSpending(double spendingAmount) {
        if (spendingAmount < 0) {
            throw std::invalid_argument("Spending amount cannot be negative");
        }
        spent_ -= spendingAmount;
        if (spent_ < 0) spent_ = 0;
        updateStatus();
    }

    void complete() { status_ = "completed"; }
    void cancel() { status_ = "cancelled"; }

private:
    void updateStatus() {
        if (status_ == "cancelled") return;
        
        if (spent_ > amount_) {
            status_ = "exceeded";
        } else if (spent_ >= amount_) {
            status_ = "completed";
        } else {
            status_ = "active";
        }
    }

    std::string id_;
    std::string tenantId_;
    std::string workspaceId_;
    std::string name_;
    std::optional<std::string> categoryId_;
    double amount_;
    double spent_;
    BudgetPeriod period_;
    std::string startDate_;
    std::string endDate_;
    bool isRecurring_;
    int alertPercentage_;
    std::string status_; // active, completed, exceeded, cancelled
    std::string createdBy_;
};

} // namespace Finance

