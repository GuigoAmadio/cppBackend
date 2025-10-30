#pragma once
#include <memory>
#include "../../repositories/AccountRepository.hpp"
#include "../../repositories/TransactionRepository.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {
namespace Utils = Core::Utils;

class DeleteAccountUseCase {
public:
    explicit DeleteAccountUseCase(
        std::shared_ptr<AccountRepository> accountRepo,
        std::shared_ptr<TransactionRepository> transactionRepo
    ) : accountRepo_(accountRepo), transactionRepo_(transactionRepo) {}

    bool execute(const std::string& accountId, const std::string& tenantId) {
        Utils::Logger::info("[DeleteAccountUseCase] Deleting account: " + accountId);

        if (accountId.empty() || tenantId.empty()) {
            throw std::invalid_argument("Account ID and Tenant ID are required");
        }

        // Check if account exists
        auto accountOpt = accountRepo_->findById(accountId, tenantId);
        if (!accountOpt.has_value()) {
            throw std::runtime_error("Account not found");
        }

        // Check if account has transactions
        auto transactions = transactionRepo_->findByAccount(accountId, tenantId);
        if (!transactions.empty()) {
            throw std::runtime_error("Cannot delete account with existing transactions. "
                                    "Please delete or move transactions first.");
        }

        // Delete account
        if (!accountRepo_->remove(accountId)) {
            throw std::runtime_error("Failed to delete account");
        }

        Utils::Logger::info("[DeleteAccountUseCase] Account deleted successfully: " + accountId);
        return true;
    }

private:
    std::shared_ptr<AccountRepository> accountRepo_;
    std::shared_ptr<TransactionRepository> transactionRepo_;
};

} // namespace Finance

