#pragma once

#include "../SubscriptionRepository.hpp"
#include "../../../../core/database/ConnectionPool.hpp"
#include <memory>

namespace Domain::Subscription {

class SubscriptionRepositoryImpl : public SubscriptionRepository {
public:
    explicit SubscriptionRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);

    bool save(const Subscription& subscription) override;
    std::optional<Subscription> findById(const std::string& id) override;
    bool remove(const std::string& id) override;

    std::vector<Subscription> findByTenant(const std::string& tenantId) override;
    std::vector<Subscription> findByCustomer(const std::string& customerId) override;
    std::vector<Subscription> findByPlan(const std::string& planId) override;
    std::vector<Subscription> findByStatus(const std::string& tenantId, const std::string& status) override;
    std::optional<Subscription> findActiveByCustomer(const std::string& customerId, const std::string& planId) override;
    
    std::vector<Subscription> findExpiringSubscriptions(const std::string& date) override;
    std::vector<Subscription> findTrialsEnding(const std::string& date) override;
    std::vector<Subscription> findPastDueSubscriptions(int daysOverdue) override;
    
    int countByTenant(const std::string& tenantId) override;
    int countActiveByTenant(const std::string& tenantId) override;
    int countByCustomer(const std::string& customerId) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    
    Subscription mapRowToSubscription(const Core::Database::QueryResult& result, int row);
    std::vector<Subscription> mapResultToSubscriptions(const Core::Database::QueryResult& result);
};

} // namespace Domain::Subscription

