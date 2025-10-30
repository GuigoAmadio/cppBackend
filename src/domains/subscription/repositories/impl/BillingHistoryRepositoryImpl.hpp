#pragma once

#include "../BillingHistoryRepository.hpp"
#include "../../../../core/database/ConnectionPool.hpp"
#include <memory>

namespace Domain::Subscription {

class BillingHistoryRepositoryImpl : public BillingHistoryRepository {
public:
    explicit BillingHistoryRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);

    bool save(const BillingHistory& billing) override;
    std::optional<BillingHistory> findById(const std::string& id) override;
    bool remove(const std::string& id) override;

    std::vector<BillingHistory> findBySubscription(const std::string& subscriptionId) override;
    std::vector<BillingHistory> findByTenant(const std::string& tenantId) override;
    std::vector<BillingHistory> findByStatus(const std::string& tenantId, const std::string& status) override;
    std::vector<BillingHistory> findByPeriod(const std::string& tenantId, const std::string& startDate, const std::string& endDate) override;
    std::vector<BillingHistory> findPendingBillings(const std::string& tenantId) override;
    std::vector<BillingHistory> findFailedBillings(const std::string& tenantId, int lastDays) override;
    
    std::optional<BillingHistory> findLastBySubscription(const std::string& subscriptionId) override;
    std::optional<BillingHistory> findLastPaidBySubscription(const std::string& subscriptionId) override;
    
    double getTotalRevenueByPeriod(const std::string& tenantId, const std::string& startDate, const std::string& endDate) override;
    int countBySubscription(const std::string& subscriptionId) override;
    int countFailedBySubscription(const std::string& subscriptionId) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    
    BillingHistory mapRowToBillingHistory(const Core::Database::QueryResult& result, int row);
    std::vector<BillingHistory> mapResultToBillingHistories(const Core::Database::QueryResult& result);
};

} // namespace Domain::Subscription

