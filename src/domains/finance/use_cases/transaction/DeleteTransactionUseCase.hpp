#pragma once
#include <memory>
#include "../../repositories/TransactionRepository.hpp"
#include "../../repositories/AccountRepository.hpp"
#include "../../entities/Transaction.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {
namespace Utils = Core::Utils;

class DeleteTransactionUseCase {
public:
    explicit DeleteTransactionUseCase(
        std::shared_ptr<TransactionRepository> transactionRepo,
        std::shared_ptr<AccountRepository> accountRepo
    ) : transactionRepo_(transactionRepo), accountRepo_(accountRepo) {}

    bool execute(const std::string& transactionId, const std::string& tenantId) {
        if (transactionId.empty() || tenantId.empty()) {
            throw std::invalid_argument("Transaction ID and Tenant ID are required");
        }

        auto txnOpt = transactionRepo_->findById(transactionId, tenantId);
        if (!txnOpt.has_value()) {
            throw std::runtime_error("Transaction not found");
        }

        auto txn = txnOpt.value();

        // Cannot delete reconciled transactions
        if (txn.getStatus().isReconciled()) {
            throw std::runtime_error("Cannot delete reconciled transactions");
        }

        // Reverse account balance if completed
        if (txn.getStatus().isCompleted()) {
            auto accountOpt = accountRepo_->findById(txn.getAccountId(), tenantId);
            if (accountOpt.has_value()) {
                auto account = accountOpt.value();
                if (txn.getType().isIncome()) {
                    account.debit(txn.getAmount());
                } else if (txn.getType().isExpense()) {
                    account.credit(txn.getAmount());
                }
                accountRepo_->update(account);
            }
        }

        if (!transactionRepo_->remove(transactionId)) {
            throw std::runtime_error("Failed to delete transaction");
        }

        return true;
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
    std::shared_ptr<AccountRepository> accountRepo_;
};

} // namespace Finance

