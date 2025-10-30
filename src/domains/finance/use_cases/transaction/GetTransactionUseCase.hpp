#pragma once
#include <memory>
#include "../../repositories/TransactionRepository.hpp"
#include "../../entities/Transaction.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {
namespace Utils = Core::Utils;

class GetTransactionUseCase {
public:
    explicit GetTransactionUseCase(std::shared_ptr<TransactionRepository> transactionRepo)
        : transactionRepo_(transactionRepo) {}

    Transaction execute(const std::string& transactionId, const std::string& tenantId) {
        if (transactionId.empty() || tenantId.empty()) {
            throw std::invalid_argument("Transaction ID and Tenant ID are required");
        }

        auto txnOpt = transactionRepo_->findById(transactionId, tenantId);
        if (!txnOpt.has_value()) {
            throw std::runtime_error("Transaction not found");
        }

        return txnOpt.value();
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
};

} // namespace Finance

