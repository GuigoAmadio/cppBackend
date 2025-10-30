#pragma once

#include <memory>
#include <string>
#include "../repositories/PaymentRepository.hpp"
#include "../../order/repositories/OrderRepository.hpp"
#include "../../../core/utils/Logger.hpp"
#include "../../../core/utils/GlobalMetrics.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Payment {

struct ProcessPaymentDTO {
    std::string paymentId;
    std::string providerPaymentId;  // ID do pagamento no provider (Stripe, PayPal, etc)
    std::string providerResponse;   // JSON response do provider
    std::string userId;             // Who is processing
};

class ProcessPaymentUseCase {
public:
    ProcessPaymentUseCase(
        std::shared_ptr<PaymentRepository> repository,
        std::shared_ptr<Domain::Order::OrderRepository> orderRepository
    ) : repository_(repository), orderRepository_(orderRepository) {}

    Payment execute(const ProcessPaymentDTO& dto) {
        Utils::Logger::info("[ProcessPaymentUseCase] Processing payment: " + dto.paymentId);

        // Find payment
        auto paymentOpt = repository_->findById(dto.paymentId);
        if (!paymentOpt) {
            throw std::runtime_error("Payment not found: " + dto.paymentId);
        }

        auto payment = *paymentOpt;

        // Validate state
        if (!payment.getStatus().isPending() && !payment.getStatus().isProcessing()) {
            throw std::runtime_error("Payment is not in a processable state");
        }

        // Update payment
        payment.setProviderPaymentId(dto.providerPaymentId);
        payment.setProviderMetadata(dto.providerResponse);
        payment.markAsCompleted();
        payment.setUpdatedBy(dto.userId);

        // Save to database
        if (!repository_->save(payment)) {
            Utils::Logger::error("[ProcessPaymentUseCase] Failed to save payment");
            throw std::runtime_error("Failed to save payment");
        }

        // Find and update CAPTURE transaction
        auto transactions = repository_->findTransactionsByPaymentId(dto.paymentId);
        for (auto& transaction : transactions) {
            if (transaction.getType().toString() == "capture" && transaction.getStatus() == "pending") {
                repository_->updateTransactionStatus(transaction.getId(), "completed");
                break;
            }
        }

        // Update Order payment_status to 'paid' (only if payment is for an order)
        if (payment.getOrderId() && !payment.getOrderId()->empty()) {
            Utils::Logger::info("[ProcessPaymentUseCase] Attempting to update Order: " + *payment.getOrderId());
            if (!orderRepository_) {
                Utils::Logger::error("[ProcessPaymentUseCase] orderRepository_ is NULL!");
            } else {
                Utils::Logger::info("[ProcessPaymentUseCase] orderRepository_ is valid, searching for order...");
                try {
                    auto orderOpt = orderRepository_->findById(*payment.getOrderId());
                    if (!orderOpt) {
                        Utils::Logger::error("[ProcessPaymentUseCase] Order not found: " + *payment.getOrderId());
                    } else {
                        Utils::Logger::info("[ProcessPaymentUseCase] Order found! Updating payment_status to 'paid'");
                        auto order = *orderOpt;
                        order.setPaymentStatus(Domain::Order::PaymentStatus("paid"));
                        bool saved = orderRepository_->save(order);
                        if (saved) {
                            Utils::Logger::info("[ProcessPaymentUseCase] ✓ Order payment_status updated to 'paid' successfully!");
                        } else {
                            Utils::Logger::error("[ProcessPaymentUseCase] ✗ Failed to save order!");
                        }
                    }
                } catch (const std::exception& e) {
                    Utils::Logger::error("[ProcessPaymentUseCase] Exception updating order: " + std::string(e.what()));
                    // Don't fail the payment if order update fails
                }
            }
        } else {
            Utils::Logger::info("[ProcessPaymentUseCase] Payment is not for an order, skipping order update");
        }

        Utils::Logger::info("[ProcessPaymentUseCase] Payment processed successfully!");
        
        // Metrics: Track payment processing
        GlobalMetrics::recordPaymentProcessed(payment.getProvider().toString(), "completed", 
                                              payment.getAmount(), payment.getCurrency());
        
        return payment;
    }

private:
    std::shared_ptr<PaymentRepository> repository_;
    std::shared_ptr<Domain::Order::OrderRepository> orderRepository_;
};

} // namespace Domain::Payment

