#pragma once

#include <memory>
#include <string>
#include "../repositories/PlanRepository.hpp"
#include "../entities/Plan.hpp"
#include "../../../core/utils/Logger.hpp"
#include <chrono>
#include <sstream>

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Subscription {

struct CreatePlanDTO {
    std::string tenantId;
    std::string name;
    std::string description;
    std::string planType;  // "basic", "pro", "enterprise"
    double price;
    std::string currency = "BRL";
    std::string billingCycle;  // "monthly", "quarterly", "yearly"
    int trialDays = 0;
    std::string features;  // JSON string
    std::string limits;    // JSON string
    bool isPublic = true;
    std::string createdBy;
};

class CreatePlanUseCase {
public:
    explicit CreatePlanUseCase(std::shared_ptr<PlanRepository> repository)
        : repository_(repository) {}

    Plan execute(const CreatePlanDTO& dto) {
        Utils::Logger::info("[CreatePlanUseCase] Creating plan: " + dto.name);

        // Validações
        if (dto.tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }
        if (dto.name.empty()) {
            throw std::invalid_argument("Plan name is required");
        }
        if (dto.price < 0) {
            throw std::invalid_argument("Price cannot be negative");
        }
        if (dto.trialDays < 0) {
            throw std::invalid_argument("Trial days cannot be negative");
        }

        // Verificar se já existe plano com mesmo nome
        auto existing = repository_->findByName(dto.tenantId, dto.name);
        if (existing) {
            throw std::runtime_error("Plan with this name already exists");
        }

        // Gerar ID único
        std::string planId = "plan-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "-" + dto.tenantId.substr(0, 8);

        // Criar entity
        Plan plan(planId, dto.tenantId, dto.name, dto.price, BillingCycle(dto.billingCycle));
        
        if (!dto.description.empty()) plan.setDescription(dto.description);
        plan.setPlanType(PlanType(dto.planType));
        plan.setCurrency(dto.currency);
        plan.setTrialDays(dto.trialDays);
        if (!dto.features.empty()) plan.setFeatures(dto.features);
        if (!dto.limits.empty()) plan.setLimits(dto.limits);
        plan.setIsPublic(dto.isPublic);
        plan.setIsActive(true);
        if (!dto.createdBy.empty()) plan.setCreatedBy(dto.createdBy);

        // Salvar no banco
        if (!repository_->save(plan)) {
            Utils::Logger::error("[CreatePlanUseCase] Failed to save plan");
            throw std::runtime_error("Failed to save plan");
        }

        Utils::Logger::info("[CreatePlanUseCase] Plan created successfully: " + planId);
        return plan;
    }

private:
    std::shared_ptr<PlanRepository> repository_;
};

} // namespace Domain::Subscription

