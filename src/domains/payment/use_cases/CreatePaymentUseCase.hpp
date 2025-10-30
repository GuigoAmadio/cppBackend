#pragma once

#include <memory>
#include <string>
#include "../repositories/PaymentRepository.hpp"
#include "../entities/Payment.hpp"
#include "../entities/Transaction.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Payment {

struct CreatePaymentDTO {
    std::string orderId;          // Optional: for order payments
    std::string subscriptionId;   // Optional: for subscription payments
    std::string customerId;
    std::string tenantId;
    double amount;
    std::string currency;  // Default: "BRL"
    std::string provider;   // "stripe", "paypal", "pix", "boleto", etc
    std::string paymentMethod; // Optional: "credit_card", "debit_card", etc
    std::string createdBy;  // User ID
};

class CreatePaymentUseCase {
public:
    explicit CreatePaymentUseCase(std::shared_ptr<PaymentRepository> repository)
        : repository_(repository) {}

    Payment execute(const CreatePaymentDTO& dto) {
        Utils::Logger::info("[CreatePaymentUseCase] Creating payment");

        // Validate - at least one of orderId or subscriptionId must be provided
        if (dto.orderId.empty() && dto.subscriptionId.empty()) {
            throw std::invalid_argument("Either Order ID or Subscription ID is required");
        }
        if (dto.customerId.empty()) {
            throw std::invalid_argument("Customer ID is required");
        }
        if (dto.tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }
        if (dto.amount <= 0) {
            throw std::invalid_argument("Amount must be greater than zero");
        }
        if (dto.provider.empty()) {
            throw std::invalid_argument("Payment provider is required");
        }

        // Generate ID
        std::string paymentId = generateSimpleUUID();

        // Create Payment entity
        Payment payment(
            paymentId,
            dto.customerId,
            dto.tenantId,
            dto.amount,
            dto.currency.empty() ? "BRL" : dto.currency,
            PaymentProvider(dto.provider),
            PaymentStatus(PaymentStatus::Type::PENDING)
        );

        // Set order or subscription
        if (!dto.orderId.empty()) {
            payment.setOrderId(dto.orderId);
        }
        if (!dto.subscriptionId.empty()) {
            payment.setSubscriptionId(dto.subscriptionId);
        }

        // Set optional fields
        if (!dto.paymentMethod.empty()) {
            payment.setPaymentMethod(dto.paymentMethod);
        }
        if (!dto.createdBy.empty()) {
            payment.setCreatedBy(dto.createdBy);
        }

        // Save to database
        if (!repository_->save(payment)) {
            Utils::Logger::error("[CreatePaymentUseCase] Failed to save payment");
            throw std::runtime_error("Failed to save payment");
        }

        // Create initial CAPTURE transaction (pending)
        std::string transactionId = generateSimpleUUID();
        Transaction transaction(
            transactionId,
            paymentId,
            dto.tenantId,
            TransactionType(TransactionType::Type::CAPTURE),
            "pending",
            dto.amount,
            dto.currency.empty() ? "BRL" : dto.currency
        );
        transaction.setCreatedBy(dto.createdBy);

        if (!repository_->saveTransaction(transaction)) {
            Utils::Logger::warning("[CreatePaymentUseCase] Failed to save transaction (non-critical)");
        }

        Utils::Logger::info("[CreatePaymentUseCase] Payment created successfully: " + paymentId);
        return payment;
    }

private:
    std::shared_ptr<PaymentRepository> repository_;

    std::string generateSimpleUUID() {
        // Simple UUID generation (Windows compatible)
        static int counter = 0;
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        return "pay-" + std::to_string(now) + "-" + std::to_string(++counter);
    }
};

} // namespace Domain::Payment

