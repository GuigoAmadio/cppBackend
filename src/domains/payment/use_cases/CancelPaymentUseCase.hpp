#pragma once

#include <memory>
#include <string>
#include "../repositories/PaymentRepository.hpp"
#include "../entities/Transaction.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Payment {

struct CancelPaymentDTO {
    std::string paymentId;
    std::string reason;
    std::string userId;
};

class CancelPaymentUseCase {
public:
    explicit CancelPaymentUseCase(std::shared_ptr<PaymentRepository> repository)
        : repository_(repository) {}

    Payment execute(const CancelPaymentDTO& dto) {
        Utils::Logger::info("[CancelPaymentUseCase] Cancelling payment: " + dto.paymentId);

        // Find payment
        auto paymentOpt = repository_->findById(dto.paymentId);
        if (!paymentOpt) {
            throw std::runtime_error("Payment not found: " + dto.paymentId);
        }

        auto payment = *paymentOpt;

        // Validate
        if (!payment.canBeCancelled()) {
            throw std::runtime_error("Payment cannot be cancelled (already completed or refunded)");
        }

        // Update payment status
        payment.markAsCancelled();
        payment.setUpdatedBy(dto.userId);

        // Save payment
        if (!repository_->save(payment)) {
            Utils::Logger::error("[CancelPaymentUseCase] Failed to save payment");
            throw std::runtime_error("Failed to save payment");
        }

        // Create CANCEL transaction
        std::string transactionId = generateSimpleUUID();
        Transaction transaction(
            transactionId,
            dto.paymentId,
            payment.getTenantId(),
            TransactionType(TransactionType::Type::CANCEL),
            "completed",
            payment.getAmount(),
            payment.getCurrency()
        );
        transaction.setReason(dto.reason);
        transaction.setCreatedBy(dto.userId);

        if (!repository_->saveTransaction(transaction)) {
            Utils::Logger::warning("[CancelPaymentUseCase] Failed to save transaction (non-critical)");
        }

        Utils::Logger::info("[CancelPaymentUseCase] Payment cancelled successfully!");
        return payment;
    }

private:
    std::shared_ptr<PaymentRepository> repository_;

    std::string generateSimpleUUID() {
        static int counter = 0;
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        return "txn-" + std::to_string(now) + "-" + std::to_string(++counter);
    }
};

} // namespace Domain::Payment

