#pragma once
#include <string>
#include <optional>
#include "../value_objects/AccountType.hpp"

namespace Finance {

class Account {
public:
    Account(
        const std::string& id,
        const std::string& tenantId,
        const std::string& name,
        const AccountType& type,
        const std::string& currency,
        double balance,
        double initialBalance,
        bool isActive,
        const std::string& createdBy
    ) : id_(id),
        tenantId_(tenantId),
        name_(name),
        type_(type),
        currency_(currency),
        balance_(balance),
        initialBalance_(initialBalance),
        isActive_(isActive),
        createdBy_(createdBy) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getWorkspaceId() const { return workspaceId_; }
    const std::string& getName() const { return name_; }
    const AccountType& getType() const { return type_; }
    const std::string& getCurrency() const { return currency_; }
    double getBalance() const { return balance_; }
    double getInitialBalance() const { return initialBalance_; }
    const std::optional<std::string>& getBankName() const { return bankName_; }
    const std::optional<std::string>& getAccountNumber() const { return accountNumber_; }
    bool isActive() const { return isActive_; }
    const std::string& getCreatedBy() const { return createdBy_; }

    // Setters
    void setWorkspaceId(const std::string& workspaceId) { workspaceId_ = workspaceId; }
    void setName(const std::string& name) {
        if (name.empty()) {
            throw std::invalid_argument("Account name cannot be empty");
        }
        name_ = name;
    }
    void setBankName(const std::string& bankName) { bankName_ = bankName; }
    void setAccountNumber(const std::string& accountNumber) { accountNumber_ = accountNumber; }
    void setActive(bool isActive) { isActive_ = isActive; }

    // Business logic
    void credit(double amount) {
        if (amount <= 0) {
            throw std::invalid_argument("Credit amount must be positive");
        }
        balance_ += amount;
    }

    void debit(double amount) {
        if (amount <= 0) {
            throw std::invalid_argument("Debit amount must be positive");
        }
        
        // Credit card can go negative
        if (!type_.isCreditCard() && balance_ < amount) {
            throw std::runtime_error("Insufficient balance");
        }
        
        balance_ -= amount;
    }

    bool canDebit(double amount) const {
        if (type_.isCreditCard()) {
            return true; // Credit cards can go negative
        }
        return balance_ >= amount;
    }

    void activate() { isActive_ = true; }
    void deactivate() { isActive_ = false; }

private:
    std::string id_;
    std::string tenantId_;
    std::string workspaceId_;
    std::string name_;
    AccountType type_;
    std::string currency_;
    double balance_;
    double initialBalance_;
    std::optional<std::string> bankName_;
    std::optional<std::string> accountNumber_;
    bool isActive_;
    std::string createdBy_;
};

} // namespace Finance

