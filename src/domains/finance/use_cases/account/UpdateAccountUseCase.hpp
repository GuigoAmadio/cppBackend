#pragma once
#include <memory>
#include <optional>
#include "../../repositories/AccountRepository.hpp"
#include "../../entities/Account.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {
namespace Utils = Core::Utils;

struct UpdateAccountDTO {
    std::string accountId;
    std::string tenantId;
    std::optional<std::string> name;
    std::optional<std::string> bankName;
    std::optional<std::string> accountNumber;
    std::optional<bool> isActive;
};

class UpdateAccountUseCase {
public:
    explicit UpdateAccountUseCase(std::shared_ptr<AccountRepository> accountRepo)
        : accountRepo_(accountRepo) {}

    bool execute(const UpdateAccountDTO& dto) {
        Utils::Logger::info("[UpdateAccountUseCase] Updating account: " + dto.accountId);

        if (dto.accountId.empty() || dto.tenantId.empty()) {
            throw std::invalid_argument("Account ID and Tenant ID are required");
        }

        // Get existing account
        auto accountOpt = accountRepo_->findById(dto.accountId, dto.tenantId);
        if (!accountOpt.has_value()) {
            throw std::runtime_error("Account not found");
        }

        Account account = accountOpt.value();

        // Update fields
        if (dto.name.has_value() && !dto.name->empty()) {
            account.setName(*dto.name);
        }
        if (dto.bankName.has_value()) {
            account.setBankName(*dto.bankName);
        }
        if (dto.accountNumber.has_value()) {
            account.setAccountNumber(*dto.accountNumber);
        }
        if (dto.isActive.has_value()) {
            account.setActive(*dto.isActive);
        }

        // Save changes
        if (!accountRepo_->update(account)) {
            throw std::runtime_error("Failed to update account");
        }

        Utils::Logger::info("[UpdateAccountUseCase] Account updated successfully: " + dto.accountId);
        return true;
    }

private:
    std::shared_ptr<AccountRepository> accountRepo_;
};

} // namespace Finance

