#pragma once
#include <memory>
#include <map>
#include "../../repositories/AccountRepository.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {
namespace Utils = Core::Utils;

struct AccountBalanceDTO {
    std::string accountId;
    std::string name;
    std::string type;
    std::string currency;
    double balance;
    double initialBalance;
};

class GetAccountBalanceUseCase {
public:
    explicit GetAccountBalanceUseCase(std::shared_ptr<AccountRepository> accountRepo)
        : accountRepo_(accountRepo) {}

    AccountBalanceDTO execute(const std::string& accountId, const std::string& tenantId) {
        Utils::Logger::info("[GetAccountBalanceUseCase] Getting balance for account: " + accountId);

        if (accountId.empty() || tenantId.empty()) {
            throw std::invalid_argument("Account ID and Tenant ID are required");
        }

        auto accountOpt = accountRepo_->findById(accountId, tenantId);
        if (!accountOpt.has_value()) {
            throw std::runtime_error("Account not found");
        }

        auto account = accountOpt.value();

        AccountBalanceDTO dto;
        dto.accountId = account.getId();
        dto.name = account.getName();
        dto.type = account.getType().toString();
        dto.currency = account.getCurrency();
        dto.balance = account.getBalance();
        dto.initialBalance = account.getInitialBalance();

        Utils::Logger::info("[GetAccountBalanceUseCase] Balance retrieved: " + std::to_string(dto.balance) + " " + dto.currency);
        return dto;
    }

    // Get total balance by currency
    std::map<std::string, double> getTotalByCurrency(const std::string& tenantId) {
        Utils::Logger::info("[GetAccountBalanceUseCase] Getting total balance by currency for tenant: " + tenantId);

        if (tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }

        std::map<std::string, double> totals;
        
        // Common currencies
        std::vector<std::string> currencies = {"BRL", "USD", "EUR", "GBP"};
        
        for (const auto& currency : currencies) {
            double total = accountRepo_->getTotalBalance(tenantId, currency);
            if (total != 0.0) {
                totals[currency] = total;
            }
        }

        return totals;
    }

private:
    std::shared_ptr<AccountRepository> accountRepo_;
};

} // namespace Finance

