#pragma once
#include <memory>
#include <string>
#include <stdexcept>
#include "../../repositories/AccountRepository.hpp"
#include "../../entities/Account.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {
namespace Utils = Core::Utils;

class GetAccountUseCase {
public:
    explicit GetAccountUseCase(std::shared_ptr<AccountRepository> accountRepo)
        : accountRepo_(accountRepo) {}

    Account execute(const std::string& accountId, const std::string& tenantId) {
        Utils::Logger::info("[GetAccountUseCase] Getting account: " + accountId);

        if (accountId.empty()) {
            throw std::invalid_argument("Account ID is required");
        }
        if (tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }

        auto accountOpt = accountRepo_->findById(accountId, tenantId);
        
        if (!accountOpt.has_value()) {
            throw std::runtime_error("Account not found");
        }

        Utils::Logger::info("[GetAccountUseCase] Account retrieved successfully: " + accountId);
        return accountOpt.value();
    }

private:
    std::shared_ptr<AccountRepository> accountRepo_;
};

} // namespace Finance

