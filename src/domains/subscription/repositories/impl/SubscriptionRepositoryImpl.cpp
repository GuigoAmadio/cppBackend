#include "SubscriptionRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"
#include <sstream>

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Subscription {

SubscriptionRepositoryImpl::SubscriptionRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {
    Utils::Logger::info("[SubscriptionRepository] Initialized");
}

bool SubscriptionRepositoryImpl::save(const Subscription& subscription) {
    Utils::Logger::info("[SubscriptionRepository::save] Saving subscription: " + subscription.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            INSERT INTO subscriptions (
                id, tenant_id, customer_id, plan_id,
                status, billing_amount, currency,
                billing_cycles_completed, failed_billing_attempts,
                started_at, trial_ends_at, current_period_start, current_period_end,
                cancelled_at, expires_at, paused_at,
                next_billing_date, last_billing_date,
                metadata, cancel_reason,
                created_by, updated_by,
                created_at, updated_at
            ) VALUES (
                $1, $2, $3, $4,
                $5, $6, $7,
                $8, $9,
                NULLIF($10,'')::timestamp, NULLIF($11,'')::timestamp, NULLIF($12,'')::timestamp, NULLIF($13,'')::timestamp,
                NULLIF($14,'')::timestamp, NULLIF($15,'')::timestamp, NULLIF($16,'')::timestamp,
                NULLIF($17,'')::timestamp, NULLIF($18,'')::timestamp,
                NULLIF($19,'')::jsonb, NULLIF($20,''),
                NULLIF($21,''), NULLIF($22,''),
                CURRENT_TIMESTAMP, CURRENT_TIMESTAMP
            )
            ON CONFLICT (id) DO UPDATE SET
                status = EXCLUDED.status,
                billing_amount = EXCLUDED.billing_amount,
                currency = EXCLUDED.currency,
                billing_cycles_completed = EXCLUDED.billing_cycles_completed,
                failed_billing_attempts = EXCLUDED.failed_billing_attempts,
                trial_ends_at = EXCLUDED.trial_ends_at,
                current_period_start = EXCLUDED.current_period_start,
                current_period_end = EXCLUDED.current_period_end,
                cancelled_at = EXCLUDED.cancelled_at,
                expires_at = EXCLUDED.expires_at,
                paused_at = EXCLUDED.paused_at,
                next_billing_date = EXCLUDED.next_billing_date,
                last_billing_date = EXCLUDED.last_billing_date,
                metadata = EXCLUDED.metadata,
                cancel_reason = EXCLUDED.cancel_reason,
                updated_by = EXCLUDED.updated_by,
                updated_at = CURRENT_TIMESTAMP
        )";
        
        std::vector<std::string> params = {
            subscription.getId(),
            subscription.getTenantId(),
            subscription.getCustomerId(),
            subscription.getPlanId(),
            subscription.getStatus().toString(),
            std::to_string(subscription.getBillingAmount()),
            subscription.getCurrency(),
            std::to_string(subscription.getBillingCyclesCompleted()),
            std::to_string(subscription.getFailedBillingAttempts()),
            subscription.getStartedAt().value_or(""),
            subscription.getTrialEndsAt().value_or(""),
            subscription.getCurrentPeriodStart().value_or(""),
            subscription.getCurrentPeriodEnd().value_or(""),
            subscription.getCancelledAt().value_or(""),
            subscription.getExpiresAt().value_or(""),
            subscription.getPausedAt().value_or(""),
            subscription.getNextBillingDate().value_or(""),
            subscription.getLastBillingDate().value_or(""),
            subscription.getMetadata().value_or(""),
            subscription.getCancelReason().value_or(""),
            subscription.getCreatedBy().value_or(""),
            subscription.getUpdatedBy().value_or("")
        };
        
        auto result = conn->executeParams(query, params);
        
        if (!result.isSuccess()) {
            Utils::Logger::error("[SubscriptionRepository::save] Failed: " + result.getError());
            return false;
        }
        
        Utils::Logger::info("[SubscriptionRepository::save] Subscription saved successfully!");
        return true;
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::save] Exception: " + std::string(e.what()));
        return false;
    }
}

std::optional<Subscription> SubscriptionRepositoryImpl::findById(const std::string& id) {
    Utils::Logger::info("[SubscriptionRepository::findById] Finding: " + id);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, customer_id, plan_id,
                   status, billing_amount, currency,
                   billing_cycles_completed, failed_billing_attempts,
                   started_at, trial_ends_at, current_period_start, current_period_end,
                   cancelled_at, expires_at, paused_at,
                   next_billing_date, last_billing_date,
                   metadata, cancel_reason,
                   created_by, updated_by, created_at, updated_at
            FROM subscriptions
            WHERE id = $1
        )";
        
        auto result = conn->executeParams(query, {id});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            Utils::Logger::info("[SubscriptionRepository::findById] Not found");
            return std::nullopt;
        }
        
        Utils::Logger::info("[SubscriptionRepository::findById] Found!");
        return mapRowToSubscription(result, 0);
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::findById] Exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool SubscriptionRepositoryImpl::remove(const std::string& id) {
    Utils::Logger::info("[SubscriptionRepository::remove] Removing: " + id);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "DELETE FROM subscriptions WHERE id = $1";
        auto result = conn->executeParams(query, {id});
        
        if (!result.isSuccess()) {
            Utils::Logger::error("[SubscriptionRepository::remove] Failed");
            return false;
        }
        
        Utils::Logger::info("[SubscriptionRepository::remove] Removed successfully!");
        return true;
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::remove] Exception: " + std::string(e.what()));
        return false;
    }
}

std::vector<Subscription> SubscriptionRepositoryImpl::findByTenant(const std::string& tenantId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, customer_id, plan_id,
                   status, billing_amount, currency,
                   billing_cycles_completed, failed_billing_attempts,
                   started_at, trial_ends_at, current_period_start, current_period_end,
                   cancelled_at, expires_at, paused_at,
                   next_billing_date, last_billing_date,
                   metadata, cancel_reason,
                   created_by, updated_by, created_at, updated_at
            FROM subscriptions
            WHERE tenant_id = $1
            ORDER BY created_at DESC
        )";
        
        auto result = conn->executeParams(query, {tenantId});
        return result.isSuccess() ? mapResultToSubscriptions(result) : std::vector<Subscription>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::findByTenant] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Subscription> SubscriptionRepositoryImpl::findByCustomer(const std::string& customerId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, customer_id, plan_id,
                   status, billing_amount, currency,
                   billing_cycles_completed, failed_billing_attempts,
                   started_at, trial_ends_at, current_period_start, current_period_end,
                   cancelled_at, expires_at, paused_at,
                   next_billing_date, last_billing_date,
                   metadata, cancel_reason,
                   created_by, updated_by, created_at, updated_at
            FROM subscriptions
            WHERE customer_id = $1
            ORDER BY created_at DESC
        )";
        
        auto result = conn->executeParams(query, {customerId});
        return result.isSuccess() ? mapResultToSubscriptions(result) : std::vector<Subscription>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::findByCustomer] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Subscription> SubscriptionRepositoryImpl::findByPlan(const std::string& planId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, customer_id, plan_id,
                   status, billing_amount, currency,
                   billing_cycles_completed, failed_billing_attempts,
                   started_at, trial_ends_at, current_period_start, current_period_end,
                   cancelled_at, expires_at, paused_at,
                   next_billing_date, last_billing_date,
                   metadata, cancel_reason,
                   created_by, updated_by, created_at, updated_at
            FROM subscriptions
            WHERE plan_id = $1
            ORDER BY created_at DESC
        )";
        
        auto result = conn->executeParams(query, {planId});
        return result.isSuccess() ? mapResultToSubscriptions(result) : std::vector<Subscription>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::findByPlan] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Subscription> SubscriptionRepositoryImpl::findByStatus(const std::string& tenantId, const std::string& status) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, customer_id, plan_id,
                   status, billing_amount, currency,
                   billing_cycles_completed, failed_billing_attempts,
                   started_at, trial_ends_at, current_period_start, current_period_end,
                   cancelled_at, expires_at, paused_at,
                   next_billing_date, last_billing_date,
                   metadata, cancel_reason,
                   created_by, updated_by, created_at, updated_at
            FROM subscriptions
            WHERE tenant_id = $1 AND status = $2
            ORDER BY created_at DESC
        )";
        
        auto result = conn->executeParams(query, {tenantId, status});
        return result.isSuccess() ? mapResultToSubscriptions(result) : std::vector<Subscription>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::findByStatus] Exception: " + std::string(e.what()));
        return {};
    }
}

std::optional<Subscription> SubscriptionRepositoryImpl::findActiveByCustomer(const std::string& customerId, const std::string& planId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, customer_id, plan_id,
                   status, billing_amount, currency,
                   billing_cycles_completed, failed_billing_attempts,
                   started_at, trial_ends_at, current_period_start, current_period_end,
                   cancelled_at, expires_at, paused_at,
                   next_billing_date, last_billing_date,
                   metadata, cancel_reason,
                   created_by, updated_by, created_at, updated_at
            FROM subscriptions
            WHERE customer_id = $1 AND plan_id = $2 AND status IN ('trial', 'active')
            LIMIT 1
        )";
        
        auto result = conn->executeParams(query, {customerId, planId});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return std::nullopt;
        }
        
        return mapRowToSubscription(result, 0);
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::findActiveByCustomer] Exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<Subscription> SubscriptionRepositoryImpl::findExpiringSubscriptions(const std::string& date) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, customer_id, plan_id,
                   status, billing_amount, currency,
                   billing_cycles_completed, failed_billing_attempts,
                   started_at, trial_ends_at, current_period_start, current_period_end,
                   cancelled_at, expires_at, paused_at,
                   next_billing_date, last_billing_date,
                   metadata, cancel_reason,
                   created_by, updated_by, created_at, updated_at
            FROM subscriptions
            WHERE status IN ('active', 'trial') 
              AND next_billing_date <= $1::timestamp
            ORDER BY next_billing_date ASC
        )";
        
        auto result = conn->executeParams(query, {date});
        return result.isSuccess() ? mapResultToSubscriptions(result) : std::vector<Subscription>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::findExpiringSubscriptions] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Subscription> SubscriptionRepositoryImpl::findTrialsEnding(const std::string& date) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, customer_id, plan_id,
                   status, billing_amount, currency,
                   billing_cycles_completed, failed_billing_attempts,
                   started_at, trial_ends_at, current_period_start, current_period_end,
                   cancelled_at, expires_at, paused_at,
                   next_billing_date, last_billing_date,
                   metadata, cancel_reason,
                   created_by, updated_by, created_at, updated_at
            FROM subscriptions
            WHERE status = 'trial' 
              AND trial_ends_at <= $1::timestamp
            ORDER BY trial_ends_at ASC
        )";
        
        auto result = conn->executeParams(query, {date});
        return result.isSuccess() ? mapResultToSubscriptions(result) : std::vector<Subscription>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::findTrialsEnding] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Subscription> SubscriptionRepositoryImpl::findPastDueSubscriptions(int daysOverdue) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, tenant_id, customer_id, plan_id,
                   status, billing_amount, currency,
                   billing_cycles_completed, failed_billing_attempts,
                   started_at, trial_ends_at, current_period_start, current_period_end,
                   cancelled_at, expires_at, paused_at,
                   next_billing_date, last_billing_date,
                   metadata, cancel_reason,
                   created_by, updated_by, created_at, updated_at
            FROM subscriptions
            WHERE status = 'past_due'
              AND next_billing_date < (CURRENT_TIMESTAMP - INTERVAL '1 day' * $1)
            ORDER BY next_billing_date ASC
        )";
        
        auto result = conn->executeParams(query, {std::to_string(daysOverdue)});
        return result.isSuccess() ? mapResultToSubscriptions(result) : std::vector<Subscription>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::findPastDueSubscriptions] Exception: " + std::string(e.what()));
        return {};
    }
}

int SubscriptionRepositoryImpl::countByTenant(const std::string& tenantId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT COUNT(*) FROM subscriptions WHERE tenant_id = $1";
        auto result = conn->executeParams(query, {tenantId});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return 0;
        }
        
        return std::stoi(result.getValue(0, 0));
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::countByTenant] Exception: " + std::string(e.what()));
        return 0;
    }
}

int SubscriptionRepositoryImpl::countActiveByTenant(const std::string& tenantId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT COUNT(*) FROM subscriptions WHERE tenant_id = $1 AND status IN ('active', 'trial')";
        auto result = conn->executeParams(query, {tenantId});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return 0;
        }
        
        return std::stoi(result.getValue(0, 0));
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::countActiveByTenant] Exception: " + std::string(e.what()));
        return 0;
    }
}

int SubscriptionRepositoryImpl::countByCustomer(const std::string& customerId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT COUNT(*) FROM subscriptions WHERE customer_id = $1";
        auto result = conn->executeParams(query, {customerId});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return 0;
        }
        
        return std::stoi(result.getValue(0, 0));
    } catch (const std::exception& e) {
        Utils::Logger::error("[SubscriptionRepository::countByCustomer] Exception: " + std::string(e.what()));
        return 0;
    }
}

// Private helper methods
Subscription SubscriptionRepositoryImpl::mapRowToSubscription(const Core::Database::QueryResult& result, int row) {
    Subscription sub(
        result.getValue(row, 0),  // id
        result.getValue(row, 1),  // tenant_id
        result.getValue(row, 2),  // customer_id
        result.getValue(row, 3),  // plan_id
        std::stod(result.getValue(row, 5))  // billing_amount
    );
    
    sub.setStatus(SubscriptionStatus(result.getValue(row, 4)));
    sub.setCurrency(result.getValue(row, 6));
    sub.setBillingCyclesCompleted(std::stoi(result.getValue(row, 7)));
    sub.setFailedBillingAttempts(std::stoi(result.getValue(row, 8)));
    
    if (!result.getValue(row, 9).empty()) sub.setStartedAt(result.getValue(row, 9));
    if (!result.getValue(row, 10).empty()) sub.setTrialEndsAt(result.getValue(row, 10));
    if (!result.getValue(row, 11).empty()) sub.setCurrentPeriodStart(result.getValue(row, 11));
    if (!result.getValue(row, 12).empty()) sub.setCurrentPeriodEnd(result.getValue(row, 12));
    if (!result.getValue(row, 13).empty()) sub.setCancelledAt(result.getValue(row, 13));
    if (!result.getValue(row, 14).empty()) sub.setExpiresAt(result.getValue(row, 14));
    if (!result.getValue(row, 15).empty()) sub.setPausedAt(result.getValue(row, 15));
    if (!result.getValue(row, 16).empty()) sub.setNextBillingDate(result.getValue(row, 16));
    if (!result.getValue(row, 17).empty()) sub.setLastBillingDate(result.getValue(row, 17));
    if (!result.getValue(row, 18).empty()) sub.setMetadata(result.getValue(row, 18));
    if (!result.getValue(row, 19).empty()) sub.setCancelReason(result.getValue(row, 19));
    if (!result.getValue(row, 20).empty()) sub.setCreatedBy(result.getValue(row, 20));
    if (!result.getValue(row, 21).empty()) sub.setUpdatedBy(result.getValue(row, 21));
    if (!result.getValue(row, 22).empty()) sub.setCreatedAt(result.getValue(row, 22));
    if (!result.getValue(row, 23).empty()) sub.setUpdatedAt(result.getValue(row, 23));
    
    return sub;
}

std::vector<Subscription> SubscriptionRepositoryImpl::mapResultToSubscriptions(const Core::Database::QueryResult& result) {
    std::vector<Subscription> subscriptions;
    for (int i = 0; i < result.rowCount(); ++i) {
        subscriptions.push_back(mapRowToSubscription(result, i));
    }
    return subscriptions;
}

} // namespace Domain::Subscription

