#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "../entities/Budget.hpp"

namespace Finance {

class BudgetRepository {
public:
    virtual ~BudgetRepository() = default;

    virtual bool save(const Budget& budget) = 0;
    virtual bool update(const Budget& budget) = 0;
    virtual bool remove(const std::string& id) = 0;
    
    virtual std::optional<Budget> findById(const std::string& id, const std::string& tenantId) = 0;
    virtual std::vector<Budget> findByTenant(const std::string& tenantId) = 0;
    virtual std::vector<Budget> findByCategory(const std::string& categoryId, const std::string& tenantId) = 0;
    virtual std::vector<Budget> findByPeriod(const std::string& period, const std::string& tenantId) = 0;
    virtual std::vector<Budget> findActive(const std::string& tenantId) = 0;
    virtual std::vector<Budget> findActiveForDate(const std::string& date, const std::string& tenantId) = 0;
    
    virtual bool updateSpent(const std::string& id, double newSpent) = 0;
};

} // namespace Finance

