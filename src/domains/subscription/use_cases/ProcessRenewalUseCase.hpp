#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "../repositories/SubscriptionRepository.hpp"
#include "../repositories/PlanRepository.hpp"
#include "../repositories/BillingHistoryRepository.hpp"
#include "../../payment/repositories/PaymentRepository.hpp"
#include "../../payment/use_cases/CreatePaymentUseCase.hpp"
#include "../../payment/use_cases/ProcessPaymentUseCase.hpp"
#include "../entities/Subscription.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Subscription {

struct ProcessRenewalDTO {
    std::string subscriptionId;
    std::string processedBy = "system";  // "system" ou user_id
};

class ProcessRenewalUseCase {
public:
    ProcessRenewalUseCase(
        std::shared_ptr<SubscriptionRepository> subscriptionRepo,
        std::shared_ptr<PlanRepository> planRepo,
        std::shared_ptr<BillingHistoryRepository> billingRepo,
        std::shared_ptr<Domain::Payment::PaymentRepository> paymentRepo
    ) : subscriptionRepo_(subscriptionRepo),
        planRepo_(planRepo),
        billingRepo_(billingRepo),
        paymentRepo_(paymentRepo) {}

    Subscription execute(const ProcessRenewalDTO& dto) {
        Utils::Logger::info("[ProcessRenewalUseCase] Processing renewal for: " + dto.subscriptionId);

        // Buscar subscription
        auto subOpt = subscriptionRepo_->findById(dto.subscriptionId);
        if (!subOpt) {
            throw std::runtime_error("Subscription not found");
        }
        auto subscription = *subOpt;

        // Verificar se pode ser cobrada
        if (!subscription.canBeBilled()) {
            throw std::runtime_error("Subscription cannot be billed in current state: " + subscription.getStatus().toString());
        }

        // Buscar plano
        auto planOpt = planRepo_->findById(subscription.getPlanId());
        if (!planOpt) {
            throw std::runtime_error("Plan not found");
        }
        auto& plan = *planOpt;

        // Se estava em trial, ativar
        if (subscription.getStatus().isTrial()) {
            subscription.activate();
            Utils::Logger::info("[ProcessRenewalUseCase] Subscription converted from trial to active");
        }

        // Calcular próximo período
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        std::string billingDate = formatTimestamp(nowTime);

        auto periodEnd = now + std::chrono::hours(24 * 30 * plan.getBillingCycle().getMonths());
        std::string nextPeriodEnd = formatTimestamp(std::chrono::system_clock::to_time_t(periodEnd));

        // Criar billing history
        std::string billingId = "bill-" + std::to_string(now.time_since_epoch().count()) + "-" + 
                               std::to_string(subscription.getBillingCyclesCompleted() + 1);
        
        BillingHistory billing(
            billingId,
            subscription.getId(),
            subscription.getTenantId(),
            subscription.getBillingAmount()
        );
        billing.setCurrency(subscription.getCurrency());
        billing.setBillingDate(billingDate);
        billing.setPeriodStart(billingDate);
        billing.setPeriodEnd(nextPeriodEnd);
        billing.setStatus(BillingStatus(BillingStatus::Type::PENDING));

        if (!billingRepo_->save(billing)) {
            Utils::Logger::error("[ProcessRenewalUseCase] Failed to save billing history");
            throw std::runtime_error("Failed to create billing record");
        }

        // Criar pagamento (integração com Payment Module)
        try {
            // Criar payment
            Domain::Payment::CreatePaymentDTO paymentDto;
            paymentDto.subscriptionId = subscription.getId();  // Usar subscription ID
            paymentDto.customerId = subscription.getCustomerId();
            paymentDto.tenantId = subscription.getTenantId();
            paymentDto.amount = subscription.getBillingAmount();
            paymentDto.currency = subscription.getCurrency();
            paymentDto.provider = "subscription";  // Provider específico para renovações automáticas
            paymentDto.paymentMethod = "subscription";
            paymentDto.createdBy = dto.processedBy;

            // Criar use case e executar
            Domain::Payment::CreatePaymentUseCase createPaymentUseCase(paymentRepo_);
            auto payment = createPaymentUseCase.execute(paymentDto);

            Utils::Logger::info("[ProcessRenewalUseCase] Payment created: " + payment.getId());

            // Tentar processar pagamento imediatamente
            Domain::Payment::ProcessPaymentDTO processDto;
            processDto.paymentId = payment.getId();
            processDto.providerPaymentId = "AUTO-RENEWAL-" + std::to_string(now.time_since_epoch().count());
            processDto.providerResponse = R"({"status": "approved", "method": "subscription"})";
            processDto.userId = dto.processedBy;

            Domain::Payment::ProcessPaymentUseCase processPaymentUseCase(paymentRepo_, nullptr);  // nullptr para orderRepo (não precisa atualizar order)
            auto processedPayment = processPaymentUseCase.execute(processDto);

            Utils::Logger::info("[ProcessRenewalUseCase] Payment processed successfully");

            // Atualizar billing history como pago
            billing.markAsPaid(payment.getId(), "subscription");
            billingRepo_->save(billing);

            // Atualizar subscription
            subscription.setCurrentPeriodStart(billingDate);
            subscription.setCurrentPeriodEnd(nextPeriodEnd);
            subscription.setNextBillingDate(nextPeriodEnd);
            subscription.setLastBillingDate(billingDate);
            subscription.incrementBillingCycle();
            subscription.resetFailedAttempts();
            subscription.setUpdatedBy(dto.processedBy);

            if (!subscriptionRepo_->save(subscription)) {
                Utils::Logger::error("[ProcessRenewalUseCase] Failed to update subscription");
                throw std::runtime_error("Failed to update subscription");
            }

            Utils::Logger::info("[ProcessRenewalUseCase] Renewal processed successfully!");
            return subscription;

        } catch (const std::exception& e) {
            Utils::Logger::error("[ProcessRenewalUseCase] Payment failed: " + std::string(e.what()));

            // Marcar billing como falho
            billing.markAsFailed(e.what());
            billingRepo_->save(billing);

            // Atualizar subscription como past_due
            subscription.incrementFailedAttempts();
            subscription.markAsPastDue();
            subscription.setUpdatedBy(dto.processedBy);
            subscriptionRepo_->save(subscription);

            throw std::runtime_error("Renewal failed: Payment processing error");
        }
    }

private:
    std::shared_ptr<SubscriptionRepository> subscriptionRepo_;
    std::shared_ptr<PlanRepository> planRepo_;
    std::shared_ptr<BillingHistoryRepository> billingRepo_;
    std::shared_ptr<Domain::Payment::PaymentRepository> paymentRepo_;

    std::string formatTimestamp(std::time_t time) {
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

} // namespace Domain::Subscription

