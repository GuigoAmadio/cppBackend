#pragma once
#include <memory>
#include "../BudgetRepository.hpp"
#include "../../../../core/database/ConnectionPool.hpp"

namespace Finance {

class BudgetRepositoryImpl : public BudgetRepository {
public:
    explicit BudgetRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);

    bool save(const Budget& budget) override;
    bool update(const Budget& budget) override;
    bool remove(const std::string& id) override;
    
    std::optional<Budget> findById(const std::string& id, const std::string& tenantId) override;
    std::vector<Budget> findByTenant(const std::string& tenantId) override;
    std::vector<Budget> findByCategory(const std::string& categoryId, const std::string& tenantId) override;
    std::vector<Budget> findByPeriod(const std::string& period, const std::string& tenantId) override;
    std::vector<Budget> findActive(const std::string& tenantId) override;
    std::vector<Budget> findActiveForDate(const std::string& date, const std::string& tenantId) override;
    
    bool updateSpent(const std::string& id, double newSpent) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    Budget mapToBudget(const Core::Database::QueryResult& result, int row);
};

} // namespace Finance

