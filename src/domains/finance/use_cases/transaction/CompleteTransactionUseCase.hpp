#pragma once
#include <memory>
#include "../../repositories/TransactionRepository.hpp"
#include "../../repositories/AccountRepository.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {
namespace Utils = Core::Utils;

class CompleteTransactionUseCase {
public:
    explicit CompleteTransactionUseCase(
        std::shared_ptr<TransactionRepository> transactionRepo,
        std::shared_ptr<AccountRepository> accountRepo
    ) : transactionRepo_(transactionRepo), accountRepo_(accountRepo) {}

    bool execute(const std::string& transactionId, const std::string& tenantId) {
        auto txnOpt = transactionRepo_->findById(transactionId, tenantId);
        if (!txnOpt.has_value()) throw std::runtime_error("Transaction not found");

        auto txn = txnOpt.value();
        if (txn.getStatus().isCompleted()) throw std::runtime_error("Transaction already completed");

        // Update account balance
        auto accountOpt = accountRepo_->findById(txn.getAccountId(), tenantId);
        if (accountOpt.has_value()) {
            auto account = accountOpt.value();
            if (txn.getType().isIncome()) account.credit(txn.getAmount());
            else if (txn.getType().isExpense()) account.debit(txn.getAmount());
            accountRepo_->update(account);
        }

        txn.complete();
        return transactionRepo_->update(txn);
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
    std::shared_ptr<AccountRepository> accountRepo_;
};

class CancelTransactionUseCase {
public:
    explicit CancelTransactionUseCase(
        std::shared_ptr<TransactionRepository> transactionRepo,
        std::shared_ptr<AccountRepository> accountRepo
    ) : transactionRepo_(transactionRepo), accountRepo_(accountRepo) {}

    bool execute(const std::string& transactionId, const std::string& tenantId) {
        auto txnOpt = transactionRepo_->findById(transactionId, tenantId);
        if (!txnOpt.has_value()) throw std::runtime_error("Transaction not found");

        auto txn = txnOpt.value();
        if (txn.getStatus().isCancelled()) throw std::runtime_error("Transaction already cancelled");
        if (txn.getStatus().isReconciled()) throw std::runtime_error("Cannot cancel reconciled transaction");

        // Reverse balance if was completed
        if (txn.getStatus().isCompleted()) {
            auto accountOpt = accountRepo_->findById(txn.getAccountId(), tenantId);
            if (accountOpt.has_value()) {
                auto account = accountOpt.value();
                if (txn.getType().isIncome()) account.debit(txn.getAmount());
                else if (txn.getType().isExpense()) account.credit(txn.getAmount());
                accountRepo_->update(account);
            }
        }

        txn.cancel();
        return transactionRepo_->update(txn);
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
    std::shared_ptr<AccountRepository> accountRepo_;
};

} // namespace Finance

