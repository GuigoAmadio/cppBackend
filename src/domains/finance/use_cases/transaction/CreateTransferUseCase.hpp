#pragma once
#include <memory>
#include "../../repositories/TransactionRepository.hpp"
#include "../../repositories/AccountRepository.hpp"
#include "../../entities/Transaction.hpp"
#include "../../../../core/utils/Logger.hpp"
#include "../../../../core/utils/IdGenerator.hpp"

namespace Finance {
namespace Utils = Core::Utils;

struct CreateTransferDTO {
    std::string tenantId;
    std::string fromAccountId;
    std::string toAccountId;
    double amount;
    std::string description;
    std::string transactionDate;
    std::string createdBy;
};

class CreateTransferUseCase {
public:
    explicit CreateTransferUseCase(
        std::shared_ptr<TransactionRepository> transactionRepo,
        std::shared_ptr<AccountRepository> accountRepo
    ) : transactionRepo_(transactionRepo), accountRepo_(accountRepo) {}

    std::string execute(const CreateTransferDTO& dto) {
        Utils::Logger::info("[CreateTransferUseCase] Creating transfer from " + dto.fromAccountId + " to " + dto.toAccountId);

        // Validations
        if (dto.tenantId.empty()) throw std::invalid_argument("Tenant ID is required");
        if (dto.fromAccountId.empty()) throw std::invalid_argument("From Account ID is required");
        if (dto.toAccountId.empty()) throw std::invalid_argument("To Account ID is required");
        if (dto.fromAccountId == dto.toAccountId) throw std::invalid_argument("Cannot transfer to the same account");
        if (dto.amount <= 0) throw std::invalid_argument("Amount must be positive");
        if (dto.transactionDate.empty()) throw std::invalid_argument("Transaction date is required");
        if (dto.createdBy.empty()) throw std::invalid_argument("Created by is required");

        // Get both accounts
        auto fromAccountOpt = accountRepo_->findById(dto.fromAccountId, dto.tenantId);
        auto toAccountOpt = accountRepo_->findById(dto.toAccountId, dto.tenantId);
        
        if (!fromAccountOpt.has_value()) throw std::runtime_error("From account not found");
        if (!toAccountOpt.has_value()) throw std::runtime_error("To account not found");

        auto fromAccount = fromAccountOpt.value();
        auto toAccount = toAccountOpt.value();

        // Check if currencies match
        if (fromAccount.getCurrency() != toAccount.getCurrency()) {
            throw std::invalid_argument("Transfer requires accounts with matching currencies");
        }

        // Check balance
        if (!fromAccount.canDebit(dto.amount)) {
            throw std::runtime_error("Insufficient balance in source account");
        }

        // Generate IDs for both transactions (usando gerador robusto)
        std::string transferId = Core::Utils::IdGenerator::generate("tfr");
        std::string txnId1 = Core::Utils::IdGenerator::generate("txn");
        std::string txnId2 = Core::Utils::IdGenerator::generate("txn");

        // Create OUTBOUND transaction (from source account)
        Transaction txnOut(
            txnId1, dto.tenantId, dto.fromAccountId,
            TransactionType(TransactionType::Type::TRANSFER),
            dto.amount, fromAccount.getCurrency(),
            TransactionStatus(TransactionStatus::Type::COMPLETED),
            dto.transactionDate, dto.createdBy
        );
        txnOut.setFromAccountId(dto.fromAccountId);
        txnOut.setToAccountId(dto.toAccountId);
        if (!dto.description.empty()) txnOut.setDescription(dto.description + " (Outbound)");

        // Create INBOUND transaction (to destination account)
        Transaction txnIn(
            txnId2, dto.tenantId, dto.toAccountId,
            TransactionType(TransactionType::Type::TRANSFER),
            dto.amount, toAccount.getCurrency(),
            TransactionStatus(TransactionStatus::Type::COMPLETED),
            dto.transactionDate, dto.createdBy
        );
        txnIn.setFromAccountId(dto.fromAccountId);
        txnIn.setToAccountId(dto.toAccountId);
        if (!dto.description.empty()) txnIn.setDescription(dto.description + " (Inbound)");

        // Update account balances
        fromAccount.debit(dto.amount);
        toAccount.credit(dto.amount);

        // Save transactions
        if (!transactionRepo_->save(txnOut)) throw std::runtime_error("Failed to save outbound transaction");
        if (!transactionRepo_->save(txnIn)) throw std::runtime_error("Failed to save inbound transaction");

        // Update accounts
        if (!accountRepo_->update(fromAccount)) throw std::runtime_error("Failed to update source account");
        if (!accountRepo_->update(toAccount)) throw std::runtime_error("Failed to update destination account");

        Utils::Logger::info("[CreateTransferUseCase] Transfer completed: " + transferId);
        return transferId;
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
    std::shared_ptr<AccountRepository> accountRepo_;
};

} // namespace Finance

