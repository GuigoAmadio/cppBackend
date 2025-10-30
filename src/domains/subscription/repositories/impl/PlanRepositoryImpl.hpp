#pragma once

#include "../PlanRepository.hpp"
#include "../../../../core/database/ConnectionPool.hpp"
#include <memory>

namespace Domain::Subscription {

class PlanRepositoryImpl : public PlanRepository {
public:
    explicit PlanRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);

    bool save(const Plan& plan) override;
    std::optional<Plan> findById(const std::string& id) override;
    bool remove(const std::string& id) override;

    std::vector<Plan> findByTenant(const std::string& tenantId) override;
    std::vector<Plan> findActivePlans(const std::string& tenantId) override;
    std::vector<Plan> findPublicPlans(const std::string& tenantId) override;
    std::vector<Plan> findByType(const std::string& tenantId, const std::string& planType) override;
    std::optional<Plan> findByName(const std::string& tenantId, const std::string& name) override;
    
    int countByTenant(const std::string& tenantId) override;
    int countActivePlans(const std::string& tenantId) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    
    Plan mapRowToPlan(const Core::Database::QueryResult& result, int row);
    std::vector<Plan> mapResultToPlans(const Core::Database::QueryResult& result);
};

} // namespace Domain::Subscription

