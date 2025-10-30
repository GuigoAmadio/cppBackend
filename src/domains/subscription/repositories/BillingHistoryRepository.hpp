#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>
#include "../entities/BillingHistory.hpp"

namespace Domain::Subscription {

class BillingHistoryRepository {
public:
    virtual ~BillingHistoryRepository() = default;

    // CRUD básico
    virtual bool save(const BillingHistory& billing) = 0;
    virtual std::optional<BillingHistory> findById(const std::string& id) = 0;
    virtual bool remove(const std::string& id) = 0;

    // Queries específicas
    virtual std::vector<BillingHistory> findBySubscription(const std::string& subscriptionId) = 0;
    virtual std::vector<BillingHistory> findByTenant(const std::string& tenantId) = 0;
    virtual std::vector<BillingHistory> findByStatus(const std::string& tenantId, const std::string& status) = 0;
    virtual std::vector<BillingHistory> findByPeriod(const std::string& tenantId, const std::string& startDate, const std::string& endDate) = 0;
    virtual std::vector<BillingHistory> findPendingBillings(const std::string& tenantId) = 0;
    virtual std::vector<BillingHistory> findFailedBillings(const std::string& tenantId, int lastDays) = 0;
    
    // Últimas cobranças
    virtual std::optional<BillingHistory> findLastBySubscription(const std::string& subscriptionId) = 0;
    virtual std::optional<BillingHistory> findLastPaidBySubscription(const std::string& subscriptionId) = 0;
    
    // Estatísticas
    virtual double getTotalRevenueByPeriod(const std::string& tenantId, const std::string& startDate, const std::string& endDate) = 0;
    virtual int countBySubscription(const std::string& subscriptionId) = 0;
    virtual int countFailedBySubscription(const std::string& subscriptionId) = 0;
};

} // namespace Domain::Subscription

