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

struct RefundPaymentDTO {
    std::string paymentId;
    double amount;  // Pode ser parcial ou total
    std::string reason;
    std::string userId;
};

class RefundPaymentUseCase {
public:
    explicit RefundPaymentUseCase(std::shared_ptr<PaymentRepository> repository)
        : repository_(repository) {}

    Payment execute(const RefundPaymentDTO& dto) {
        Utils::Logger::info("[RefundPaymentUseCase] Refunding payment: " + dto.paymentId);

        // Find payment
        auto paymentOpt = repository_->findById(dto.paymentId);
        if (!paymentOpt) {
            throw std::runtime_error("Payment not found: " + dto.paymentId);
        }

        auto payment = *paymentOpt;

        // Validate
        if (!payment.canBeRefunded()) {
            throw std::runtime_error("Payment cannot be refunded (not completed)");
        }

        if (dto.amount <= 0 || dto.amount > payment.getAmount()) {
            throw std::invalid_argument("Invalid refund amount");
        }

        // Update payment status
        payment.markAsRefunded();
        payment.setUpdatedBy(dto.userId);

        // Save payment
        if (!repository_->save(payment)) {
            Utils::Logger::error("[RefundPaymentUseCase] Failed to save payment");
            throw std::runtime_error("Failed to save payment");
        }

        // Create REFUND transaction
        std::string transactionId = generateSimpleUUID();
        Transaction transaction(
            transactionId,
            dto.paymentId,
            payment.getTenantId(),
            TransactionType(TransactionType::Type::REFUND),
            "completed",
            dto.amount,
            payment.getCurrency()
        );
        transaction.setReason(dto.reason);
        transaction.setCreatedBy(dto.userId);

        if (!repository_->saveTransaction(transaction)) {
            Utils::Logger::warning("[RefundPaymentUseCase] Failed to save transaction (non-critical)");
        }

        Utils::Logger::info("[RefundPaymentUseCase] Payment refunded successfully!");
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

