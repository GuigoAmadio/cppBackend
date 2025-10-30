#pragma once
#include <memory>
#include <string>
#include "../../repositories/TransactionRepository.hpp"
#include "../../repositories/AccountRepository.hpp"
#include "../../repositories/BudgetRepository.hpp"
#include "../../entities/Transaction.hpp"
#include "../../../../core/utils/Logger.hpp"
#include "../../../../core/utils/IdGenerator.hpp"

namespace Finance {
namespace Utils = Core::Utils;

struct CreateTransactionDTO {
    std::string tenantId;
    std::string workspaceId;
    std::string accountId;
    std::string categoryId;
    std::string type;  // income, expense
    double amount;
    std::string currency;
    std::string description;
    std::string transactionDate;
    std::string referenceId;
    std::string referenceType;
    std::string createdBy;
};

class CreateTransactionUseCase {
public:
    explicit CreateTransactionUseCase(
        std::shared_ptr<TransactionRepository> transactionRepo,
        std::shared_ptr<AccountRepository> accountRepo,
        std::shared_ptr<BudgetRepository> budgetRepo
    ) : transactionRepo_(transactionRepo), 
        accountRepo_(accountRepo),
        budgetRepo_(budgetRepo) {}

    std::string execute(const CreateTransactionDTO& dto) {
        Utils::Logger::info("[CreateTransactionUseCase] Creating " + dto.type + " transaction for account: " + dto.accountId);

        // Validations
        if (dto.tenantId.empty()) throw std::invalid_argument("Tenant ID is required");
        if (dto.accountId.empty()) throw std::invalid_argument("Account ID is required");
        if (dto.type.empty()) throw std::invalid_argument("Transaction type is required");
        if (dto.amount <= 0) throw std::invalid_argument("Amount must be positive");
        if (dto.currency.empty()) throw std::invalid_argument("Currency is required");
        if (dto.transactionDate.empty()) throw std::invalid_argument("Transaction date is required");
        if (dto.createdBy.empty()) throw std::invalid_argument("Created by is required");

        // Validate account exists
        auto accountOpt = accountRepo_->findById(dto.accountId, dto.tenantId);
        if (!accountOpt.has_value()) {
            throw std::runtime_error("Account not found");
        }

        auto account = accountOpt.value();

        // Check currency match
        if (account.getCurrency() != dto.currency) {
            throw std::invalid_argument("Transaction currency must match account currency");
        }

        // Generate ID (usando gerador robusto com milissegundos + contador atômico)
        std::string id = Core::Utils::IdGenerator::generate("txn");

        // Create transaction
        Transaction transaction(
            id,
            dto.tenantId,
            dto.accountId,
            TransactionType(dto.type),
            dto.amount,
            dto.currency,
            TransactionStatus(TransactionStatus::Type::COMPLETED),  // Auto-complete
            dto.transactionDate,
            dto.createdBy
        );

        if (!dto.workspaceId.empty()) transaction.setWorkspaceId(dto.workspaceId);
        if (!dto.categoryId.empty()) transaction.setCategoryId(dto.categoryId);
        if (!dto.description.empty()) transaction.setDescription(dto.description);
        if (!dto.referenceId.empty()) transaction.setReferenceId(dto.referenceId);
        if (!dto.referenceType.empty()) transaction.setReferenceType(dto.referenceType);

        // Update account balance
        if (dto.type == "income") {
            account.credit(dto.amount);
        } else if (dto.type == "expense") {
            if (!account.canDebit(dto.amount)) {
                throw std::runtime_error("Insufficient balance in account");
            }
            account.debit(dto.amount);
        }

        // Update budget if expense and category provided
        if (dto.type == "expense" && !dto.categoryId.empty()) {
            auto budgets = budgetRepo_->findActiveForDate(dto.transactionDate, dto.tenantId);
            for (auto& budget : budgets) {
                if (budget.getCategoryId().has_value() && *budget.getCategoryId() == dto.categoryId) {
                    budget.addSpending(dto.amount);
                    budgetRepo_->updateSpent(budget.getId(), budget.getSpent());
                    Utils::Logger::info("[CreateTransactionUseCase] Budget updated: " + budget.getId());
                }
            }
        }

        // Save transaction
        std::ostringstream txnLog;
        txnLog << "[CreateTransactionUseCase] About to save transaction to repository with data: "
               << "id=" << transaction.getId()
               << ", tenantId=" << transaction.getTenantId()
               << ", accountId=" << transaction.getAccountId()
               << ", workspaceId=" << (transaction.getWorkspaceId().empty() ? "null" : transaction.getWorkspaceId())
               << ", type=" << transaction.getType().toString()
               << ", amount=" << transaction.getAmount()
               << ", currency=" << transaction.getCurrency()
               << ", status=" << transaction.getStatus().toString()
               << ", date=" << transaction.getTransactionDate()
               << ", createdBy=" << transaction.getCreatedBy()
               << ", categoryId=" << (transaction.getCategoryId().has_value() ? *transaction.getCategoryId() : "null")
               << ", description=" << (transaction.getDescription().has_value() ? *transaction.getDescription() : "null")
               << ", referenceId=" << (transaction.getReferenceId().has_value() ? *transaction.getReferenceId() : "null")
               << ", referenceType=" << (transaction.getReferenceType().has_value() ? *transaction.getReferenceType() : "null")
               << ", fromAccountId=" << (transaction.getFromAccountId().has_value() ? *transaction.getFromAccountId() : "null")
               << ", toAccountId=" << (transaction.getToAccountId().has_value() ? *transaction.getToAccountId() : "null");
        Utils::Logger::info(txnLog.str());
        bool saveResult = transactionRepo_->save(transaction);
        Utils::Logger::info("[CreateTransactionUseCase] Transaction save() returned: " + std::string(saveResult ? "TRUE" : "FALSE"));
        if (!saveResult) {
            Utils::Logger::info("[CreateTransactionUseCase] ❌ Transaction save failed!");
            throw std::runtime_error("Failed to save transaction");
        }

        // Save account with updated balance
        Utils::Logger::info("[CreateTransactionUseCase] About to update account balance...");
        bool updateResult = accountRepo_->update(account);
        Utils::Logger::info("[CreateTransactionUseCase] Account update() returned: " + std::string(updateResult ? "TRUE" : "FALSE"));
        if (!updateResult) {
            Utils::Logger::info("[CreateTransactionUseCase] ❌ Account update failed!");
            throw std::runtime_error("Failed to update account balance");
        }

        Utils::Logger::info("[CreateTransactionUseCase] Transaction created successfully: " + id);
        return id;
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
    std::shared_ptr<AccountRepository> accountRepo_;
    std::shared_ptr<BudgetRepository> budgetRepo_;
};

} // namespace Finance

