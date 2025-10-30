#pragma once
#include <memory>
#include <string>
#include <stdexcept>
#include "../../repositories/AccountRepository.hpp"
#include "../../entities/Account.hpp"
#include "../../value_objects/AccountType.hpp"
#include "../../../../core/utils/Logger.hpp"
#include "../../../../core/utils/IdGenerator.hpp"

namespace Finance {
namespace Utils = Core::Utils;

struct CreateAccountDTO {
    std::string tenantId;
    std::string workspaceId;
    std::string name;
    std::string type;
    std::string currency;
    double initialBalance;
    std::string bankName;
    std::string accountNumber;
    std::string createdBy;
};

class CreateAccountUseCase {
public:
    explicit CreateAccountUseCase(std::shared_ptr<AccountRepository> accountRepo)
        : accountRepo_(accountRepo) {}

    std::string execute(const CreateAccountDTO& dto) {
        Utils::Logger::info("[CreateAccountUseCase] Creating account: " + dto.name);

        // Validations
        if (dto.tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }
        if (dto.name.empty()) {
            throw std::invalid_argument("Account name is required");
        }
        if (dto.type.empty()) {
            throw std::invalid_argument("Account type is required");
        }
        if (dto.currency.empty()) {
            throw std::invalid_argument("Currency is required");
        }
        if (dto.currency.length() != 3) {
            throw std::invalid_argument("Currency must be a 3-letter code (e.g., BRL, USD, EUR)");
        }
        if (dto.createdBy.empty()) {
            throw std::invalid_argument("Created by is required");
        }

        // Validate account type
        AccountType accountType(dto.type);

        // Generate ID (usando gerador robusto com milissegundos + contador atômico)
        std::string id = Core::Utils::IdGenerator::generate("acc");

        // Create account entity
        Account account(
            id,
            dto.tenantId,
            dto.name,
            accountType,
            dto.currency,
            dto.initialBalance,  // Initial balance = current balance
            dto.initialBalance,
            true,  // isActive
            dto.createdBy
        );

        if (!dto.workspaceId.empty()) {
            account.setWorkspaceId(dto.workspaceId);
        }
        if (!dto.bankName.empty()) {
            account.setBankName(dto.bankName);
        }
        if (!dto.accountNumber.empty()) {
            account.setAccountNumber(dto.accountNumber);
        }

        // Save to repository
        Utils::Logger::info("[CreateAccountUseCase] About to save account to repository...");
        Utils::Logger::info(
            "[CreateAccountUseCase] Account details:"
            " id=" + id +
            ", tenantId=" + dto.tenantId +
            ", name=" + dto.name +
            ", type=" + dto.type +
            ", currency=" + dto.currency +
            ", initialBalance=" + std::to_string(dto.initialBalance) +
            ", createdBy=" + dto.createdBy +
            ", workspaceId=" + (dto.workspaceId.empty() ? "N/A" : dto.workspaceId) +
            ", bankName=" + (dto.bankName.empty() ? "N/A" : dto.bankName) +
            ", accountNumber=" + (dto.accountNumber.empty() ? "N/A" : dto.accountNumber)
        );
        bool saveResult = accountRepo_->save(account);
        Utils::Logger::info("[CreateAccountUseCase] Repository save() returned: " + std::string(saveResult ? "TRUE" : "FALSE"));
        
        if (!saveResult) {
            Utils::Logger::info("[CreateAccountUseCase] ❌ Save failed! Throwing exception...");
            throw std::runtime_error("Failed to save account");
        }

        Utils::Logger::info("[CreateAccountUseCase] Account created successfully: " + id);
        return id;
    }

private:
    std::shared_ptr<AccountRepository> accountRepo_;
};

} // namespace Finance

