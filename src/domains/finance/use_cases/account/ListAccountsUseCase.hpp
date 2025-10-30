#pragma once
#include <memory>
#include <vector>
#include "../../repositories/AccountRepository.hpp"
#include "../../entities/Account.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {
namespace Utils = Core::Utils;

class ListAccountsUseCase {
public:
    explicit ListAccountsUseCase(std::shared_ptr<AccountRepository> accountRepo)
        : accountRepo_(accountRepo) {}

    std::vector<Account> execute(const std::string& tenantId, const std::string& workspaceId = "",
                                 const std::string& currency = "", bool activeOnly = false) {
        Utils::Logger::info("[ListAccountsUseCase] Listing accounts for tenant: " + tenantId);

        if (tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }

        std::vector<Account> accounts;

        if (!workspaceId.empty()) {
            accounts = accountRepo_->findByWorkspace(workspaceId, tenantId);
        } else if (!currency.empty()) {
            accounts = accountRepo_->findByCurrency(currency, tenantId);
        } else if (activeOnly) {
            accounts = accountRepo_->findActive(tenantId);
        } else {
            accounts = accountRepo_->findByTenant(tenantId);
        }

        Utils::Logger::info("[ListAccountsUseCase] Found " + std::to_string(accounts.size()) + " accounts");
        return accounts;
    }

private:
    std::shared_ptr<AccountRepository> accountRepo_;
};

} // namespace Finance

