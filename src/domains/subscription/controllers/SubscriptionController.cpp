#include "SubscriptionController.hpp"
#include <sstream>

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Subscription {

using namespace Core::Http;
using namespace Core::Json;

SubscriptionController::SubscriptionController(
    std::shared_ptr<PlanRepository> planRepo,
    std::shared_ptr<SubscriptionRepository> subscriptionRepo,
    std::shared_ptr<BillingHistoryRepository> billingRepo,
    std::shared_ptr<Domain::Payment::PaymentRepository> paymentRepo
) : planRepo_(planRepo),
    subscriptionRepo_(subscriptionRepo),
    billingRepo_(billingRepo),
    paymentRepo_(paymentRepo) {
    Utils::Logger::info("[SubscriptionController] Initialized");
}

// ==================== PLAN ENDPOINTS ====================

Response SubscriptionController::createPlan(const Request& req) {
    Utils::Logger::info("[SubscriptionController::createPlan] Creating plan");
    
    auto json = makeObject();
    
    try {
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();
        
        CreatePlanDTO dto;
        dto.tenantId = req.getCustomData("user_tenant_id");
        dto.name = bodyObj["name"]->asString();
        dto.price = bodyObj["price"]->asNumber();
        dto.billingCycle = bodyObj["billing_cycle"]->asString();
        dto.planType = bodyObj.count("plan_type") ? bodyObj["plan_type"]->asString() : "basic";
        
        if (bodyObj.count("description")) dto.description = bodyObj["description"]->asString();
        if (bodyObj.count("currency")) dto.currency = bodyObj["currency"]->asString();
        if (bodyObj.count("trial_days")) dto.trialDays = static_cast<int>(bodyObj["trial_days"]->asNumber());
        if (bodyObj.count("features")) dto.features = bodyObj["features"]->toString();
        if (bodyObj.count("limits")) dto.limits = bodyObj["limits"]->toString();
        if (bodyObj.count("is_public")) dto.isPublic = bodyObj["is_public"]->asString() == "true";
        
        dto.createdBy = req.getCustomData("user_id");

        CreatePlanUseCase useCase(planRepo_);
        auto plan = useCase.execute(dto);

        json->asObject()["plan"] = makeString(planToJson(plan));
        return Response(StatusCode::Created).json(*json);
        
    } catch (const std::invalid_argument& e) {
        json->asObject()["error"] = makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionController::createPlan] Error: " + std::string(e.what()));
        json->asObject()["error"] = makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response SubscriptionController::getPlan(const Request& req) {
    auto json = makeObject();
    
    try {
        std::string planId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        auto planOpt = planRepo_->findById(planId);
        if (!planOpt) {
            json->asObject()["error"] = makeString("Plan not found");
            return Response(StatusCode::NotFound).json(*json);
        }

        auto& plan = *planOpt;
        if (plan.getTenantId() != tenantId) {
            json->asObject()["error"] = makeString("Access denied");
            return Response(StatusCode::Forbidden).json(*json);
        }

        json->asObject()["plan"] = makeString(planToJson(plan));
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionController::getPlan] Error: " + std::string(e.what()));
        json->asObject()["error"] = makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response SubscriptionController::listPlans(const Request& req) {
    auto json = makeObject();
    
    try {
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string activeOnly = req.getQuery("active");
        std::string publicOnly = req.getQuery("public");

        std::vector<Plan> plans;
        
        if (activeOnly == "true" && publicOnly == "true") {
            plans = planRepo_->findPublicPlans(tenantId);
        } else if (activeOnly == "true") {
            plans = planRepo_->findActivePlans(tenantId);
        } else {
            plans = planRepo_->findByTenant(tenantId);
        }

        auto plansArray = makeArray();
        for (const auto& plan : plans) {
            plansArray->asArray().push_back(makeString(planToJson(plan)));
        }

        json->asObject()["plans"] = std::move(plansArray);
        json->asObject()["count"] = makeNumber(plans.size());
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionController::listPlans] Error: " + std::string(e.what()));
        json->asObject()["error"] = makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// ==================== SUBSCRIPTION ENDPOINTS ====================

Response SubscriptionController::createSubscription(const Request& req) {
    Utils::Logger::info("[SubscriptionController::createSubscription] Creating subscription");
    
    auto json = makeObject();
    
    try {
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();
        
        CreateSubscriptionDTO dto;
        dto.tenantId = req.getCustomData("user_tenant_id");
        dto.customerId = bodyObj["customer_id"]->asString();
        dto.planId = bodyObj["plan_id"]->asString();
        dto.createdBy = req.getCustomData("user_id");

        CreateSubscriptionUseCase useCase(subscriptionRepo_, planRepo_, billingRepo_);
        auto subscription = useCase.execute(dto);

        json->asObject()["subscription"] = makeString(subscriptionToJson(subscription));
        return Response(StatusCode::Created).json(*json);
        
    } catch (const std::invalid_argument& e) {
        json->asObject()["error"] = makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionController::createSubscription] Error: " + std::string(e.what()));
        json->asObject()["error"] = makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response SubscriptionController::getSubscription(const Request& req) {
    auto json = makeObject();
    
    try {
        std::string subscriptionId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        GetSubscriptionDTO dto;
        dto.subscriptionId = subscriptionId;
        dto.tenantId = tenantId;

        GetSubscriptionUseCase useCase(subscriptionRepo_);
        auto subscription = useCase.execute(dto);

        json->asObject()["subscription"] = makeString(subscriptionToJson(subscription));
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionController::getSubscription] Error: " + std::string(e.what()));
        json->asObject()["error"] = makeString(e.what());
        return Response(StatusCode::NotFound).json(*json);
    }
}

Response SubscriptionController::listSubscriptions(const Request& req) {
    auto json = makeObject();
    
    try {
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string customerId = req.getQuery("customer_id");
        std::string status = req.getQuery("status");

        ListSubscriptionsDTO dto;
        dto.tenantId = tenantId;
        dto.customerId = customerId;
        dto.status = status;

        ListSubscriptionsUseCase useCase(subscriptionRepo_);
        auto subscriptions = useCase.execute(dto);

        auto subsArray = makeArray();
        for (const auto& sub : subscriptions) {
            subsArray->asArray().push_back(makeString(subscriptionToJson(sub)));
        }

        json->asObject()["subscriptions"] = std::move(subsArray);
        json->asObject()["count"] = makeNumber(subscriptions.size());
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionController::listSubscriptions] Error: " + std::string(e.what()));
        json->asObject()["error"] = makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response SubscriptionController::cancelSubscription(const Request& req) {
    Utils::Logger::info("[SubscriptionController::cancelSubscription] Cancelling subscription");
    
    auto json = makeObject();
    
    try {
        std::string subscriptionId = req.getParam("id");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();
        
        CancelSubscriptionDTO dto;
        dto.subscriptionId = subscriptionId;
        dto.reason = bodyObj.count("reason") ? bodyObj["reason"]->asString() : "Customer request";
        dto.immediate = bodyObj.count("immediate") ? (bodyObj["immediate"]->asString() == "true") : false;
        dto.cancelledBy = req.getCustomData("user_id");

        CancelSubscriptionUseCase useCase(subscriptionRepo_);
        auto subscription = useCase.execute(dto);

        json->asObject()["subscription"] = makeString(subscriptionToJson(subscription));
        json->asObject()["message"] = makeString("Subscription cancelled successfully");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionController::cancelSubscription] Error: " + std::string(e.what()));
        json->asObject()["error"] = makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    }
}

Response SubscriptionController::getBillingHistory(const Request& req) {
    auto json = makeObject();
    
    try {
        std::string subscriptionId = req.getParam("id");
        
        auto billings = billingRepo_->findBySubscription(subscriptionId);

        auto billingsArray = makeArray();
        for (const auto& billing : billings) {
            billingsArray->asArray().push_back(makeString(billingHistoryToJson(billing)));
        }

        json->asObject()["billings"] = std::move(billingsArray);
        json->asObject()["count"] = makeNumber(billings.size());
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionController::getBillingHistory] Error: " + std::string(e.what()));
        json->asObject()["error"] = makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response SubscriptionController::processRenewal(const Request& req) {
    Utils::Logger::info("[SubscriptionController::processRenewal] Processing renewal");
    
    auto json = makeObject();
    
    try {
        std::string subscriptionId = req.getParam("id");
        
        ProcessRenewalDTO dto;
        dto.subscriptionId = subscriptionId;
        dto.processedBy = req.getCustomData("user_id");
        // Sim, basta instanciar e executar o use case, o que já está sendo feito:
        ProcessRenewalUseCase useCase(subscriptionRepo_, planRepo_, billingRepo_, paymentRepo_);
        auto subscription = useCase.execute(dto);

        json->asObject()["subscription"] = makeString(subscriptionToJson(subscription));
        json->asObject()["message"] = makeString("Renewal processed successfully");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionController::processRenewal] Error: " + std::string(e.what()));
        json->asObject()["error"] = makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    }
}

Response SubscriptionController::getSubscriptionStats(const Request& req) {
    auto json = makeObject();
    
    try {
        std::string tenantId = req.getCustomData("user_tenant_id");

        int totalSubs = subscriptionRepo_->countByTenant(tenantId);
        int activeSubs = subscriptionRepo_->countActiveByTenant(tenantId);
        int totalPlans = planRepo_->countByTenant(tenantId);
        int activePlans = planRepo_->countActivePlans(tenantId);

        json->asObject()["total_subscriptions"] = makeNumber(totalSubs);
        json->asObject()["active_subscriptions"] = makeNumber(activeSubs);
        json->asObject()["total_plans"] = makeNumber(totalPlans);
        json->asObject()["active_plans"] = makeNumber(activePlans);
        
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionController::getSubscriptionStats] Error: " + std::string(e.what()));
        json->asObject()["error"] = makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// Stubs for remaining endpoints
Response SubscriptionController::updatePlan(const Request& req) {
    auto json = makeObject();
    json->asObject()["error"] = makeString("Not implemented yet");
    return Response(StatusCode::NotImplemented).json(*json);
}

Response SubscriptionController::deletePlan(const Request& req) {
    auto json = makeObject();
    json->asObject()["error"] = makeString("Not implemented yet");
    return Response(StatusCode::NotImplemented).json(*json);
}

Response SubscriptionController::pauseSubscription(const Request& req) {
    auto json = makeObject();
    json->asObject()["error"] = makeString("Not implemented yet");
    return Response(StatusCode::NotImplemented).json(*json);
}

Response SubscriptionController::resumeSubscription(const Request& req) {
    auto json = makeObject();
    json->asObject()["error"] = makeString("Not implemented yet");
    return Response(StatusCode::NotImplemented).json(*json);
}

// ==================== HELPER METHODS ====================

std::string SubscriptionController::planToJson(const Plan& plan) {
    std::ostringstream oss;
    oss << "{"
        << R"("id":")" << plan.getId() << "\","
        << R"("tenant_id":")" << plan.getTenantId() << "\","
        << R"("name":")" << plan.getName() << "\","
        << R"("plan_type":")" << plan.getPlanType().toString() << "\","
        << R"("price":)" << plan.getPrice() << ","
        << R"("currency":")" << plan.getCurrency() << "\","
        << R"("billing_cycle":")" << plan.getBillingCycle().toString() << "\","
        << R"("trial_days":)" << plan.getTrialDays() << ","
        << R"("is_active":)" << (plan.isActive() ? "true" : "false") << ","
        << R"("is_public":)" << (plan.isPublic() ? "true" : "false");
    
    if (plan.getDescription()) oss << R"(,"description":")" << *plan.getDescription() << "\"";
    
    oss << "}";
    return oss.str();
}

std::string SubscriptionController::subscriptionToJson(const Subscription& sub) {
    std::ostringstream oss;
    oss << "{"
        << R"("id":")" << sub.getId() << "\","
        << R"("tenant_id":")" << sub.getTenantId() << "\","
        << R"("customer_id":")" << sub.getCustomerId() << "\","
        << R"("plan_id":")" << sub.getPlanId() << "\","
        << R"("status":")" << sub.getStatus().toString() << "\","
        << R"("billing_amount":)" << sub.getBillingAmount() << ","
        << R"("currency":")" << sub.getCurrency() << "\","
        << R"("billing_cycles_completed":)" << sub.getBillingCyclesCompleted() << ","
        << R"("failed_billing_attempts":)" << sub.getFailedBillingAttempts();
    
    if (sub.getNextBillingDate()) oss << R"(,"next_billing_date":")" << *sub.getNextBillingDate() << "\"";
    if (sub.getCurrentPeriodEnd()) oss << R"(,"current_period_end":")" << *sub.getCurrentPeriodEnd() << "\"";
    
    oss << "}";
    return oss.str();
}

std::string SubscriptionController::billingHistoryToJson(const BillingHistory& billing) {
    std::ostringstream oss;
    oss << "{"
        << R"("id":")" << billing.getId() << "\","
        << R"("subscription_id":")" << billing.getSubscriptionId() << "\","
        << R"("amount":)" << billing.getAmount() << ","
        << R"("currency":")" << billing.getCurrency() << "\","
        << R"("status":")" << billing.getStatus().toString() << "\"";
    
    if (billing.getBillingDate()) oss << R"(,"billing_date":")" << *billing.getBillingDate() << "\"";
    if (billing.getPaymentId()) oss << R"(,"payment_id":")" << *billing.getPaymentId() << "\"";
    
    oss << "}";
    return oss.str();
}

} // namespace Domain::Subscription

