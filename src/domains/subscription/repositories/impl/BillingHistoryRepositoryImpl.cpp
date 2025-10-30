#include "BillingHistoryRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"
#include <sstream>

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Subscription {

BillingHistoryRepositoryImpl::BillingHistoryRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {
    Utils::Logger::info("[BillingHistoryRepository] Initialized");
}

bool BillingHistoryRepositoryImpl::save(const BillingHistory& billing) {
    Utils::Logger::info("[BillingHistoryRepository::save] Saving billing: " + billing.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            INSERT INTO subscription_billing_history (
                id, subscription_id, tenant_id,
                billing_date, period_start, period_end,
                amount, currency, status,
                payment_id, payment_method,
                invoice_number, invoice_url, failure_reason,
                metadata,
                created_at, updated_at
            ) VALUES (
                $1, $2, $3,
                NULLIF($4,'')::timestamp, NULLIF($5,'')::timestamp, NULLIF($6,'')::timestamp,
                $7, $8, $9,
                NULLIF($10,''), NULLIF($11,''),
                NULLIF($12,''), NULLIF($13,''), NULLIF($14,''),
                NULLIF($15,'')::jsonb,
                CURRENT_TIMESTAMP, CURRENT_TIMESTAMP
            )
            ON CONFLICT (id) DO UPDATE SET
                status = EXCLUDED.status,
                payment_id = EXCLUDED.payment_id,
                payment_method = EXCLUDED.payment_method,
                invoice_number = EXCLUDED.invoice_number,
                invoice_url = EXCLUDED.invoice_url,
                failure_reason = EXCLUDED.failure_reason,
                metadata = EXCLUDED.metadata,
                updated_at = CURRENT_TIMESTAMP
        )";
        
        std::vector<std::string> params = {
            billing.getId(),
            billing.getSubscriptionId(),
            billing.getTenantId(),
            billing.getBillingDate().value_or(""),
            billing.getPeriodStart().value_or(""),
            billing.getPeriodEnd().value_or(""),
            std::to_string(billing.getAmount()),
            billing.getCurrency(),
            billing.getStatus().toString(),
            billing.getPaymentId().value_or(""),
            billing.getPaymentMethod().value_or(""),
            billing.getInvoiceNumber().value_or(""),
            billing.getInvoiceUrl().value_or(""),
            billing.getFailureReason().value_or(""),
            billing.getMetadata().value_or("")
        };
        
        auto result = conn->executeParams(query, params);
        
        if (!result.isSuccess()) {
            Utils::Logger::error("[BillingHistoryRepository::save] Failed: " + result.getError());
            return false;
        }
        
        Utils::Logger::info("[BillingHistoryRepository::save] Billing saved successfully!");
        return true;
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::save] Exception: " + std::string(e.what()));
        return false;
    }
}

std::optional<BillingHistory> BillingHistoryRepositoryImpl::findById(const std::string& id) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, subscription_id, tenant_id,
                   billing_date, period_start, period_end,
                   amount, currency, status,
                   payment_id, payment_method,
                   invoice_number, invoice_url, failure_reason,
                   metadata, created_at, updated_at
            FROM subscription_billing_history
            WHERE id = $1
        )";
        
        auto result = conn->executeParams(query, {id});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return std::nullopt;
        }
        
        return mapRowToBillingHistory(result, 0);
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::findById] Exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool BillingHistoryRepositoryImpl::remove(const std::string& id) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = "DELETE FROM subscription_billing_history WHERE id = $1";
        auto result = conn->executeParams(query, {id});
        
        return result.isSuccess();
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::remove] Exception: " + std::string(e.what()));
        return false;
    }
}

std::vector<BillingHistory> BillingHistoryRepositoryImpl::findBySubscription(const std::string& subscriptionId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, subscription_id, tenant_id,
                   billing_date, period_start, period_end,
                   amount, currency, status,
                   payment_id, payment_method,
                   invoice_number, invoice_url, failure_reason,
                   metadata, created_at, updated_at
            FROM subscription_billing_history
            WHERE subscription_id = $1
            ORDER BY billing_date DESC
        )";
        
        auto result = conn->executeParams(query, {subscriptionId});
        return result.isSuccess() ? mapResultToBillingHistories(result) : std::vector<BillingHistory>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::findBySubscription] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<BillingHistory> BillingHistoryRepositoryImpl::findByTenant(const std::string& tenantId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, subscription_id, tenant_id,
                   billing_date, period_start, period_end,
                   amount, currency, status,
                   payment_id, payment_method,
                   invoice_number, invoice_url, failure_reason,
                   metadata, created_at, updated_at
            FROM subscription_billing_history
            WHERE tenant_id = $1
            ORDER BY billing_date DESC
        )";
        
        auto result = conn->executeParams(query, {tenantId});
        return result.isSuccess() ? mapResultToBillingHistories(result) : std::vector<BillingHistory>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::findByTenant] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<BillingHistory> BillingHistoryRepositoryImpl::findByStatus(const std::string& tenantId, const std::string& status) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, subscription_id, tenant_id,
                   billing_date, period_start, period_end,
                   amount, currency, status,
                   payment_id, payment_method,
                   invoice_number, invoice_url, failure_reason,
                   metadata, created_at, updated_at
            FROM subscription_billing_history
            WHERE tenant_id = $1 AND status = $2
            ORDER BY billing_date DESC
        )";
        
        auto result = conn->executeParams(query, {tenantId, status});
        return result.isSuccess() ? mapResultToBillingHistories(result) : std::vector<BillingHistory>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::findByStatus] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<BillingHistory> BillingHistoryRepositoryImpl::findByPeriod(const std::string& tenantId, const std::string& startDate, const std::string& endDate) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, subscription_id, tenant_id,
                   billing_date, period_start, period_end,
                   amount, currency, status,
                   payment_id, payment_method,
                   invoice_number, invoice_url, failure_reason,
                   metadata, created_at, updated_at
            FROM subscription_billing_history
            WHERE tenant_id = $1 
              AND billing_date >= $2::timestamp 
              AND billing_date <= $3::timestamp
            ORDER BY billing_date DESC
        )";
        
        auto result = conn->executeParams(query, {tenantId, startDate, endDate});
        return result.isSuccess() ? mapResultToBillingHistories(result) : std::vector<BillingHistory>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::findByPeriod] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<BillingHistory> BillingHistoryRepositoryImpl::findPendingBillings(const std::string& tenantId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, subscription_id, tenant_id,
                   billing_date, period_start, period_end,
                   amount, currency, status,
                   payment_id, payment_method,
                   invoice_number, invoice_url, failure_reason,
                   metadata, created_at, updated_at
            FROM subscription_billing_history
            WHERE tenant_id = $1 AND status = 'pending'
            ORDER BY billing_date ASC
        )";
        
        auto result = conn->executeParams(query, {tenantId});
        return result.isSuccess() ? mapResultToBillingHistories(result) : std::vector<BillingHistory>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::findPendingBillings] Exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<BillingHistory> BillingHistoryRepositoryImpl::findFailedBillings(const std::string& tenantId, int lastDays) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, subscription_id, tenant_id,
                   billing_date, period_start, period_end,
                   amount, currency, status,
                   payment_id, payment_method,
                   invoice_number, invoice_url, failure_reason,
                   metadata, created_at, updated_at
            FROM subscription_billing_history
            WHERE tenant_id = $1 
              AND status = 'failed'
              AND billing_date >= (CURRENT_TIMESTAMP - INTERVAL '1 day' * $2)
            ORDER BY billing_date DESC
        )";
        
        auto result = conn->executeParams(query, {tenantId, std::to_string(lastDays)});
        return result.isSuccess() ? mapResultToBillingHistories(result) : std::vector<BillingHistory>{};
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::findFailedBillings] Exception: " + std::string(e.what()));
        return {};
    }
}

std::optional<BillingHistory> BillingHistoryRepositoryImpl::findLastBySubscription(const std::string& subscriptionId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, subscription_id, tenant_id,
                   billing_date, period_start, period_end,
                   amount, currency, status,
                   payment_id, payment_method,
                   invoice_number, invoice_url, failure_reason,
                   metadata, created_at, updated_at
            FROM subscription_billing_history
            WHERE subscription_id = $1
            ORDER BY billing_date DESC
            LIMIT 1
        )";
        
        auto result = conn->executeParams(query, {subscriptionId});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return std::nullopt;
        }
        
        return mapRowToBillingHistory(result, 0);
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::findLastBySubscription] Exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::optional<BillingHistory> BillingHistoryRepositoryImpl::findLastPaidBySubscription(const std::string& subscriptionId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT id, subscription_id, tenant_id,
                   billing_date, period_start, period_end,
                   amount, currency, status,
                   payment_id, payment_method,
                   invoice_number, invoice_url, failure_reason,
                   metadata, created_at, updated_at
            FROM subscription_billing_history
            WHERE subscription_id = $1 AND status = 'paid'
            ORDER BY billing_date DESC
            LIMIT 1
        )";
        
        auto result = conn->executeParams(query, {subscriptionId});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return std::nullopt;
        }
        
        return mapRowToBillingHistory(result, 0);
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::findLastPaidBySubscription] Exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

double BillingHistoryRepositoryImpl::getTotalRevenueByPeriod(const std::string& tenantId, const std::string& startDate, const std::string& endDate) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT COALESCE(SUM(amount), 0) as total
            FROM subscription_billing_history
            WHERE tenant_id = $1 
              AND status = 'paid'
              AND billing_date >= $2::timestamp 
              AND billing_date <= $3::timestamp
        )";
        
        auto result = conn->executeParams(query, {tenantId, startDate, endDate});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return 0.0;
        }
        
        return std::stod(result.getValue(0, 0));
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::getTotalRevenueByPeriod] Exception: " + std::string(e.what()));
        return 0.0;
    }
}

int BillingHistoryRepositoryImpl::countBySubscription(const std::string& subscriptionId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT COUNT(*) FROM subscription_billing_history WHERE subscription_id = $1";
        auto result = conn->executeParams(query, {subscriptionId});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return 0;
        }
        
        return std::stoi(result.getValue(0, 0));
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::countBySubscription] Exception: " + std::string(e.what()));
        return 0;
    }
}

int BillingHistoryRepositoryImpl::countFailedBySubscription(const std::string& subscriptionId) {
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT COUNT(*) FROM subscription_billing_history WHERE subscription_id = $1 AND status = 'failed'";
        auto result = conn->executeParams(query, {subscriptionId});
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            return 0;
        }
        
        return std::stoi(result.getValue(0, 0));
    } catch (const std::exception& e) {
        Utils::Logger::error("[BillingHistoryRepository::countFailedBySubscription] Exception: " + std::string(e.what()));
        return 0;
    }
}

// Private helper methods
BillingHistory BillingHistoryRepositoryImpl::mapRowToBillingHistory(const Core::Database::QueryResult& result, int row) {
    BillingHistory billing(
        result.getValue(row, 0),  // id
        result.getValue(row, 1),  // subscription_id
        result.getValue(row, 2),  // tenant_id
        std::stod(result.getValue(row, 6))  // amount
    );
    
    if (!result.getValue(row, 3).empty()) billing.setBillingDate(result.getValue(row, 3));
    if (!result.getValue(row, 4).empty()) billing.setPeriodStart(result.getValue(row, 4));
    if (!result.getValue(row, 5).empty()) billing.setPeriodEnd(result.getValue(row, 5));
    billing.setCurrency(result.getValue(row, 7));
    billing.setStatus(BillingStatus(result.getValue(row, 8)));
    if (!result.getValue(row, 9).empty()) billing.setPaymentId(result.getValue(row, 9));
    if (!result.getValue(row, 10).empty()) billing.setPaymentMethod(result.getValue(row, 10));
    if (!result.getValue(row, 11).empty()) billing.setInvoiceNumber(result.getValue(row, 11));
    if (!result.getValue(row, 12).empty()) billing.setInvoiceUrl(result.getValue(row, 12));
    if (!result.getValue(row, 13).empty()) billing.setFailureReason(result.getValue(row, 13));
    if (!result.getValue(row, 14).empty()) billing.setMetadata(result.getValue(row, 14));
    if (!result.getValue(row, 15).empty()) billing.setCreatedAt(result.getValue(row, 15));
    if (!result.getValue(row, 16).empty()) billing.setUpdatedAt(result.getValue(row, 16));
    
    return billing;
}

std::vector<BillingHistory> BillingHistoryRepositoryImpl::mapResultToBillingHistories(const Core::Database::QueryResult& result) {
    std::vector<BillingHistory> billings;
    for (int i = 0; i < result.rowCount(); ++i) {
        billings.push_back(mapRowToBillingHistory(result, i));
    }
    return billings;
}

} // namespace Domain::Subscription

