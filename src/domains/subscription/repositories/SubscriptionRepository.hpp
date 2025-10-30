#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>
#include "../entities/Subscription.hpp"

namespace Domain::Subscription {

class SubscriptionRepository {
public:
    virtual ~SubscriptionRepository() = default;

    // CRUD básico
    virtual bool save(const Subscription& subscription) = 0;
    virtual std::optional<Subscription> findById(const std::string& id) = 0;
    virtual bool remove(const std::string& id) = 0;

    // Queries específicas
    virtual std::vector<Subscription> findByTenant(const std::string& tenantId) = 0;
    virtual std::vector<Subscription> findByCustomer(const std::string& customerId) = 0;
    virtual std::vector<Subscription> findByPlan(const std::string& planId) = 0;
    virtual std::vector<Subscription> findByStatus(const std::string& tenantId, const std::string& status) = 0;
    virtual std::optional<Subscription> findActiveByCustomer(const std::string& customerId, const std::string& planId) = 0;
    
    // Queries para renovação automática
    virtual std::vector<Subscription> findExpiringSubscriptions(const std::string& date) = 0;
    virtual std::vector<Subscription> findTrialsEnding(const std::string& date) = 0;
    virtual std::vector<Subscription> findPastDueSubscriptions(int daysOverdue) = 0;
    
    // Contadores
    virtual int countByTenant(const std::string& tenantId) = 0;
    virtual int countActiveByTenant(const std::string& tenantId) = 0;
    virtual int countByCustomer(const std::string& customerId) = 0;
};

} // namespace Domain::Subscription

