#pragma once
#include <memory>
#include <optional>
#include "../../repositories/TransactionRepository.hpp"
#include "../../entities/Transaction.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {
namespace Utils = Core::Utils;

struct UpdateTransactionDTO {
    std::string transactionId;
    std::string tenantId;
    std::optional<std::string> categoryId;
    std::optional<std::string> description;
};

class UpdateTransactionUseCase {
public:
    explicit UpdateTransactionUseCase(std::shared_ptr<TransactionRepository> transactionRepo)
        : transactionRepo_(transactionRepo) {}

    bool execute(const UpdateTransactionDTO& dto) {
        if (dto.transactionId.empty() || dto.tenantId.empty()) {
            throw std::invalid_argument("Transaction ID and Tenant ID are required");
        }

        auto txnOpt = transactionRepo_->findById(dto.transactionId, dto.tenantId);
        if (!txnOpt.has_value()) {
            throw std::runtime_error("Transaction not found");
        }

        Transaction txn = txnOpt.value();

        if (dto.categoryId.has_value()) txn.setCategoryId(*dto.categoryId);
        if (dto.description.has_value()) txn.setDescription(*dto.description);

        if (!transactionRepo_->update(txn)) {
            throw std::runtime_error("Failed to update transaction");
        }

        return true;
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
};

} // namespace Finance

