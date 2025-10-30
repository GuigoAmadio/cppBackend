#pragma once
#include <memory>
#include <vector>
#include "../../repositories/TransactionRepository.hpp"
#include "../../entities/Transaction.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {
namespace Utils = Core::Utils;

struct ListTransactionsFilters {
    std::string accountId;
    std::string categoryId;
    std::string status;
    std::string startDate;
    std::string endDate;
    std::string type;
};

class ListTransactionsUseCase {
public:
    explicit ListTransactionsUseCase(std::shared_ptr<TransactionRepository> transactionRepo)
        : transactionRepo_(transactionRepo) {}

    std::vector<Transaction> execute(const std::string& tenantId, const ListTransactionsFilters& filters) {
        Utils::Logger::info("[ListTransactionsUseCase] Listing transactions for tenant: " + tenantId);

        if (tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }

        std::vector<Transaction> transactions;

        // Apply filters
        if (!filters.accountId.empty()) {
            transactions = transactionRepo_->findByAccount(filters.accountId, tenantId);
        } else if (!filters.categoryId.empty()) {
            transactions = transactionRepo_->findByCategory(filters.categoryId, tenantId);
        } else if (!filters.status.empty()) {
            transactions = transactionRepo_->findByStatus(filters.status, tenantId);
        } else if (!filters.startDate.empty() && !filters.endDate.empty()) {
            transactions = transactionRepo_->findByDateRange(filters.startDate, filters.endDate, tenantId);
        } else {
            transactions = transactionRepo_->findByTenant(tenantId);
        }

        // Additional type filter
        if (!filters.type.empty()) {
            std::vector<Transaction> filtered;
            for (const auto& txn : transactions) {
                if (txn.getType().toString() == filters.type) {
                    filtered.push_back(txn);
                }
            }
            transactions = filtered;
        }

        Utils::Logger::info("[ListTransactionsUseCase] Found " + std::to_string(transactions.size()) + " transactions");
        return transactions;
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
};

} // namespace Finance

