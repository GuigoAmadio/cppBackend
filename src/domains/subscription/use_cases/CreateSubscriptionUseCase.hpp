#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "../repositories/SubscriptionRepository.hpp"
#include "../repositories/PlanRepository.hpp"
#include "../repositories/BillingHistoryRepository.hpp"
#include "../entities/Subscription.hpp"
#include "../entities/BillingHistory.hpp"
#include "../../../core/utils/Logger.hpp"
#include "../../../core/utils/GlobalMetrics.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Subscription {

struct CreateSubscriptionDTO {
    std::string tenantId;
    std::string customerId;
    std::string planId;
    std::string createdBy;
};

class CreateSubscriptionUseCase {
public:
    CreateSubscriptionUseCase(
        std::shared_ptr<SubscriptionRepository> subscriptionRepo,
        std::shared_ptr<PlanRepository> planRepo,
        std::shared_ptr<BillingHistoryRepository> billingRepo
    ) : subscriptionRepo_(subscriptionRepo), 
        planRepo_(planRepo),
        billingRepo_(billingRepo) {}

    Subscription execute(const CreateSubscriptionDTO& dto) {
        Utils::Logger::info("[CreateSubscriptionUseCase] Creating subscription for customer: " + dto.customerId);

        // Validações
        if (dto.tenantId.empty()) throw std::invalid_argument("Tenant ID is required");
        if (dto.customerId.empty()) throw std::invalid_argument("Customer ID is required");
        if (dto.planId.empty()) throw std::invalid_argument("Plan ID is required");

        // Buscar plano
        auto planOpt = planRepo_->findById(dto.planId);
        if (!planOpt) {
            throw std::runtime_error("Plan not found: " + dto.planId);
        }
        auto& plan = *planOpt;

        if (!plan.isActive()) {
            throw std::runtime_error("Plan is not active");
        }

        // Verificar se cliente já tem assinatura ativa deste plano
        auto existing = subscriptionRepo_->findActiveByCustomer(dto.customerId, dto.planId);
        if (existing) {
            throw std::runtime_error("Customer already has an active subscription for this plan");
        }

        // Gerar ID único
        std::string subId = "sub-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "-" + dto.customerId.substr(0, 8);

        // Calcular datas
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        
        std::string startedAt = formatTimestamp(nowTime);
        std::string trialEndsAt;
        std::string currentPeriodStart = startedAt;
        std::string currentPeriodEnd;
        std::string nextBillingDate;

        if (plan.hasTrialPeriod()) {
            // Tem período de trial
            auto trialEnd = now + std::chrono::hours(24 * plan.getTrialDays());
            trialEndsAt = formatTimestamp(std::chrono::system_clock::to_time_t(trialEnd));
            nextBillingDate = trialEndsAt;
            
            auto periodEnd = trialEnd + std::chrono::hours(24 * 30 * plan.getBillingCycle().getMonths());
            currentPeriodEnd = formatTimestamp(std::chrono::system_clock::to_time_t(periodEnd));
        } else {
            // Sem trial, cobrança imediata
            auto periodEnd = now + std::chrono::hours(24 * 30 * plan.getBillingCycle().getMonths());
            currentPeriodEnd = formatTimestamp(std::chrono::system_clock::to_time_t(periodEnd));
            nextBillingDate = formatTimestamp(std::chrono::system_clock::to_time_t(periodEnd));
        }

        // Criar subscription
        Subscription subscription(
            subId,
            dto.tenantId,
            dto.customerId,
            dto.planId,
            plan.getPrice()
        );

        subscription.setStatus(plan.hasTrialPeriod() ? 
            SubscriptionStatus(SubscriptionStatus::Type::TRIAL) : 
            SubscriptionStatus(SubscriptionStatus::Type::ACTIVE));
        subscription.setCurrency(plan.getCurrency());
        subscription.setStartedAt(startedAt);
        if (!trialEndsAt.empty()) subscription.setTrialEndsAt(trialEndsAt);
        subscription.setCurrentPeriodStart(currentPeriodStart);
        subscription.setCurrentPeriodEnd(currentPeriodEnd);
        subscription.setNextBillingDate(nextBillingDate);
        if (!dto.createdBy.empty()) subscription.setCreatedBy(dto.createdBy);

        // Salvar subscription
        if (!subscriptionRepo_->save(subscription)) {
            Utils::Logger::error("[CreateSubscriptionUseCase] Failed to save subscription");
            throw std::runtime_error("Failed to save subscription");
        }

        // Criar billing history (primeira cobrança)
        std::string billingId = "bill-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "-1";
        BillingHistory billing(billingId, subId, dto.tenantId, plan.getPrice());
        billing.setCurrency(plan.getCurrency());
        billing.setBillingDate(startedAt);
        billing.setPeriodStart(currentPeriodStart);
        billing.setPeriodEnd(currentPeriodEnd);
        billing.setStatus(plan.hasTrialPeriod() ? 
            BillingStatus(BillingStatus::Type::PENDING) : 
            BillingStatus(BillingStatus::Type::PENDING));

        if (!billingRepo_->save(billing)) {
            Utils::Logger::error("[CreateSubscriptionUseCase] Failed to save billing history");
            // Não falha a subscription por causa disso
        }

        Utils::Logger::info("[CreateSubscriptionUseCase] Subscription created successfully: " + subId);
        
        // Metrics: Track subscription creation
        GlobalMetrics::recordSubscriptionCreated(dto.planId, dto.tenantId);
        
        return subscription;
    }

private:
    std::shared_ptr<SubscriptionRepository> subscriptionRepo_;
    std::shared_ptr<PlanRepository> planRepo_;
    std::shared_ptr<BillingHistoryRepository> billingRepo_;

    std::string formatTimestamp(std::time_t time) {
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

} // namespace Domain::Subscription

