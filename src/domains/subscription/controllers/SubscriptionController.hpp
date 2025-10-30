#pragma once

#include <memory>
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../../../core/json/Json.hpp"
#include "../../../core/utils/Logger.hpp"

#include "../repositories/PlanRepository.hpp"
#include "../repositories/SubscriptionRepository.hpp"
#include "../repositories/BillingHistoryRepository.hpp"
#include "../../payment/repositories/PaymentRepository.hpp"

#include "../use_cases/CreatePlanUseCase.hpp"
#include "../use_cases/CreateSubscriptionUseCase.hpp"
#include "../use_cases/CancelSubscriptionUseCase.hpp"
#include "../use_cases/ProcessRenewalUseCase.hpp"
#include "../use_cases/GetSubscriptionUseCase.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Subscription {

class SubscriptionController {
public:
    SubscriptionController(
        std::shared_ptr<PlanRepository> planRepo,
        std::shared_ptr<SubscriptionRepository> subscriptionRepo,
        std::shared_ptr<BillingHistoryRepository> billingRepo,
        std::shared_ptr<Domain::Payment::PaymentRepository> paymentRepo
    );

    // Plan endpoints
    Core::Http::Response createPlan(const Core::Http::Request& req);
    Core::Http::Response getPlan(const Core::Http::Request& req);
    Core::Http::Response listPlans(const Core::Http::Request& req);
    Core::Http::Response updatePlan(const Core::Http::Request& req);
    Core::Http::Response deletePlan(const Core::Http::Request& req);

    // Subscription endpoints
    Core::Http::Response createSubscription(const Core::Http::Request& req);
    Core::Http::Response getSubscription(const Core::Http::Request& req);
    Core::Http::Response listSubscriptions(const Core::Http::Request& req);
    Core::Http::Response cancelSubscription(const Core::Http::Request& req);
    Core::Http::Response pauseSubscription(const Core::Http::Request& req);
    Core::Http::Response resumeSubscription(const Core::Http::Request& req);
    
    // Billing endpoints
    Core::Http::Response getBillingHistory(const Core::Http::Request& req);
    Core::Http::Response processRenewal(const Core::Http::Request& req);
    
    // Stats endpoints
    Core::Http::Response getSubscriptionStats(const Core::Http::Request& req);

private:
    std::shared_ptr<PlanRepository> planRepo_;
    std::shared_ptr<SubscriptionRepository> subscriptionRepo_;
    std::shared_ptr<BillingHistoryRepository> billingRepo_;
    std::shared_ptr<Domain::Payment::PaymentRepository> paymentRepo_;

    std::string planToJson(const Plan& plan);
    std::string subscriptionToJson(const Subscription& subscription);
    std::string billingHistoryToJson(const BillingHistory& billing);
};

} // namespace Domain::Subscription

