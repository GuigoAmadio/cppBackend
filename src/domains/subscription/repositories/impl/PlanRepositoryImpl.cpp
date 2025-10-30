#include "PlanRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"
#include <sstream>

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Subscription {

PlanRepositoryImpl::PlanRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {
    Utils::Logger::info("[PlanRepository] Initialized");
}

bool PlanRepositoryImpl::save(const Plan& plan) {
    Utils::Logger::info("[PlanRepository::save] Saving plan: " + plan.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            INSERT INTO subscription_plans (
                id, tenant_id, name, description, plan_type,
                price, currency, billing_cycle, trial_days,
                features, limits,
                is_active, is_public, display_order,
                created_by, updated_by,
                created_at, updated_at
            ) VALUES (
                $1, $2, $3, NULLIF($4,''), $5,
                $6, $7, $8, $9,
                NULLIF($10,'')::jsonb, NULLIF($11,'')::jsonb,
                $12, $13, $14,
                NULLIF($15,''), NULLIF($16,''),
                CURRENT_TIMESTAMP, CURRENT_TIMESTAMP
            )
            ON CONFLICT (id) DO UPDATE SET
                name = EXCLUDED.name,
                description = EXCLUDED.description,
                plan_type = EXCLUDED.plan_type,
                price = EXCLUDED.price,
                currency = EXCLUDED.currency,
                billing_cycle = EXCLUDED.billing_cycle,
                trial_days = EXCLUDED.trial_days,
                features = EXCLUDED.features,
                limits = EXCLUDED.limits,
                is_active = EXCLUDED.is_active,
                is_public = EXCLUDED.is_public,
                display_order = EXCLUDED.display_order,
                updated_by = EXCLUDED.updated_by,
                updated_at = CURRENT_TIMESTAMP
        )";
        
        std::vector<std::string> params = {
            plan.getId(),
            plan.getTenantId(),
            plan.getName(),
            plan.getDescription().value_or(""),
            plan.getPlanType().toString(),
            std::to_string(plan.getPrice()),
            plan.getCurrency(),
            plan.getBillingCycle().toString(),
            std::to_string(plan.getTrialDays()),
            plan.getFeatures().value_or(""),
            plan.getLimits().value_or(""),
            plan.isActive() ? "true" : "false",
            plan.isPublic() ? "true" : "false",
            std::to_string(plan.getDisplayOrder()),
            plan.getCreatedBy().value_or(""),
            plan.getUpdatedBy().value_or("")
        };
        
        auto result = conn->executeParams(query, params);
        
        if (!result.isSuccess()) {
            Utils::Logger::error("[PlanRepository::save] Failed to save plan: " + result.getError());
            return false;
        }
        
        Utils::Logger::info("[PlanRepository::save] Plan saved successfully!");
        return true;
    } catch (const std::exception& e) {
        Utils::Logger::error("[PlanRepository::save] Exception: " + std::string(e.what()));
        return false;
    }
}

std::optional<Plan> PlanRepositoryImpl::findById(const std::string& id) {
    Utils::Logger::info("[PlanRepository::findById] Finding plan: " + id);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, name, description, plan_type,
                   price, currency, billing_cycle, trial_days,
                   features, limits,
                   is_active, is_public, display_order,
                   created_by, updated_by, created_at, updated_at
            FROM subscription_plans
            WHERE id = $1
        )";
        
        auto result = conn->executeParams(query, {id});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            Utils::Logger::info("[PlanRepository::findById] Plan not found");
            return std::nullopt;
        }
        
        Utils::Logger::info("[PlanRepository::findById] Plan found!");
        return mapRowToPlan(result, 0);
    } catch (const std::exception& e) {
        Utils::Logger::error("[PlanRepository::findById] Exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool PlanRepositoryImpl::remove(const std::string& id) {
    Utils::Logger::info("[PlanRepository::remove] Removing plan: " + id);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "DELETE FROM subscription_plans WHERE id = $1";
        auto result = conn->executeParams(query, {id});
        
        if (!result.isSuccess()) {
            Utils::Logger::error("[PlanRepository::remove] Failed to remove plan");
            return false;
        }
        
        Utils::Logger::info("[PlanRepository::remove] Plan removed successfully!");
        return true;
    } catch (const std::exception& e) {
        Utils::Logger::error("[PlanRepository::remove] Exception: " + std::string(e.what()));
        return false;
    }
}

std::vector<Plan> PlanRepositoryImpl::findByTenant(const std::string& tenantId) {
    Utils::Logger::info("[PlanRepository::findByTenant] Finding plans for tenant: " + tenantId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, name, description, plan_type,
                   price, currency, billing_cycle, trial_days,
                   features, limits,
                   is_active, is_public, display_order,
                   created_by, updated_by, created_at, updated_at
            FROM subscription_plans
            WHERE tenant_id = $1
            ORDER BY display_order ASC, name ASC
        )";
        
        auto result = conn->executeParams(query, {tenantId});
        
        if (!result.isSuccess()) {
            Utils::Logger::error("[PlanRepository::findByTenant] Query failed");
            return {};
        }
        
        auto plans = mapResultToPlans(result);
        Utils::Logger::info("[PlanRepository::findByTenant] Found " + std::to_string(plans.size()) + " plan(s)");
        return plans;
    } catch (const std::exception& e) {
        Utils::Logger::error("[PlanRepository::findByTenant] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Plan> PlanRepositoryImpl::findActivePlans(const std::string& tenantId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, name, description, plan_type,
                   price, currency, billing_cycle, trial_days,
                   features, limits,
                   is_active, is_public, display_order,
                   created_by, updated_by, created_at, updated_at
            FROM subscription_plans
            WHERE tenant_id = $1 AND is_active = true
            ORDER BY display_order ASC, name ASC
        )";
        
        auto result = conn->executeParams(query, {tenantId});
        return result.isSuccess() ? mapResultToPlans(result) : std::vector<Plan>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[PlanRepository::findActivePlans] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Plan> PlanRepositoryImpl::findPublicPlans(const std::string& tenantId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, name, description, plan_type,
                   price, currency, billing_cycle, trial_days,
                   features, limits,
                   is_active, is_public, display_order,
                   created_by, updated_by, created_at, updated_at
            FROM subscription_plans
            WHERE tenant_id = $1 AND is_active = true AND is_public = true
            ORDER BY display_order ASC, name ASC
        )";
        
        auto result = conn->executeParams(query, {tenantId});
        return result.isSuccess() ? mapResultToPlans(result) : std::vector<Plan>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[PlanRepository::findPublicPlans] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Plan> PlanRepositoryImpl::findByType(const std::string& tenantId, const std::string& planType) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, name, description, plan_type,
                   price, currency, billing_cycle, trial_days,
                   features, limits,
                   is_active, is_public, display_order,
                   created_by, updated_by, created_at, updated_at
            FROM subscription_plans
            WHERE tenant_id = $1 AND plan_type = $2
            ORDER BY display_order ASC, name ASC
        )";
        
        auto result = conn->executeParams(query, {tenantId, planType});
        return result.isSuccess() ? mapResultToPlans(result) : std::vector<Plan>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[PlanRepository::findByType] Exception: " + std::string(e.what()));
        return {};
    }
}

std::optional<Plan> PlanRepositoryImpl::findByName(const std::string& tenantId, const std::string& name) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, name, description, plan_type,
                   price, currency, billing_cycle, trial_days,
                   features, limits,
                   is_active, is_public, display_order,
                   created_by, updated_by, created_at, updated_at
            FROM subscription_plans
            WHERE tenant_id = $1 AND name = $2
            LIMIT 1
        )";
        
        auto result = conn->executeParams(query, {tenantId, name});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return std::nullopt;
        }
        
        return mapRowToPlan(result, 0);
    } catch (const std::exception& e) {
        Utils::Logger::error("[PlanRepository::findByName] Exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

int PlanRepositoryImpl::countByTenant(const std::string& tenantId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT COUNT(*) FROM subscription_plans WHERE tenant_id = $1";
        auto result = conn->executeParams(query, {tenantId});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return 0;
        }
        
        return std::stoi(result.getValue(0, 0));
    } catch (const std::exception& e) {
        Utils::Logger::error("[PlanRepository::countByTenant] Exception: " + std::string(e.what()));
        return 0;
    }
}

int PlanRepositoryImpl::countActivePlans(const std::string& tenantId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT COUNT(*) FROM subscription_plans WHERE tenant_id = $1 AND is_active = true";
        auto result = conn->executeParams(query, {tenantId});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return 0;
        }
        
        return std::stoi(result.getValue(0, 0));
    } catch (const std::exception& e) {
        Utils::Logger::error("[PlanRepository::countActivePlans] Exception: " + std::string(e.what()));
        return 0;
    }
}

// Private helper methods
Plan PlanRepositoryImpl::mapRowToPlan(const Core::Database::QueryResult& result, int row) {
    Plan plan(
        result.getValue(row, 0),  // id
        result.getValue(row, 1),  // tenant_id
        result.getValue(row, 2),  // name
        std::stod(result.getValue(row, 5)),  // price
        BillingCycle(result.getValue(row, 7))  // billing_cycle
    );
    
    if (!result.getValue(row, 3).empty()) plan.setDescription(result.getValue(row, 3));
    plan.setPlanType(PlanType(result.getValue(row, 4)));
    plan.setCurrency(result.getValue(row, 6));
    plan.setTrialDays(std::stoi(result.getValue(row, 8)));
    if (!result.getValue(row, 9).empty()) plan.setFeatures(result.getValue(row, 9));
    if (!result.getValue(row, 10).empty()) plan.setLimits(result.getValue(row, 10));
    plan.setIsActive(result.getValue(row, 11) == "t" || result.getValue(row, 11) == "true");
    plan.setIsPublic(result.getValue(row, 12) == "t" || result.getValue(row, 12) == "true");
    plan.setDisplayOrder(std::stoi(result.getValue(row, 13)));
    if (!result.getValue(row, 14).empty()) plan.setCreatedBy(result.getValue(row, 14));
    if (!result.getValue(row, 15).empty()) plan.setUpdatedBy(result.getValue(row, 15));
    if (!result.getValue(row, 16).empty()) plan.setCreatedAt(result.getValue(row, 16));
    if (!result.getValue(row, 17).empty()) plan.setUpdatedAt(result.getValue(row, 17));
    
    return plan;
}

std::vector<Plan> PlanRepositoryImpl::mapResultToPlans(const Core::Database::QueryResult& result) {
    std::vector<Plan> plans;
    for (int i = 0; i < result.rowCount(); ++i) {
        plans.push_back(mapRowToPlan(result, i));
    }
    return plans;
}

} // namespace Domain::Subscription

