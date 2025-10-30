#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>
#include "../entities/Plan.hpp"

namespace Domain::Subscription {

class PlanRepository {
public:
    virtual ~PlanRepository() = default;

    // CRUD básico
    virtual bool save(const Plan& plan) = 0;
    virtual std::optional<Plan> findById(const std::string& id) = 0;
    virtual bool remove(const std::string& id) = 0;

    // Queries específicas
    virtual std::vector<Plan> findByTenant(const std::string& tenantId) = 0;
    virtual std::vector<Plan> findActivePlans(const std::string& tenantId) = 0;
    virtual std::vector<Plan> findPublicPlans(const std::string& tenantId) = 0;
    virtual std::vector<Plan> findByType(const std::string& tenantId, const std::string& planType) = 0;
    virtual std::optional<Plan> findByName(const std::string& tenantId, const std::string& name) = 0;
    
    // Contadores
    virtual int countByTenant(const std::string& tenantId) = 0;
    virtual int countActivePlans(const std::string& tenantId) = 0;
};

} // namespace Domain::Subscription

