#include "BudgetRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {

namespace Utils = Core::Utils;

BudgetRepositoryImpl::BudgetRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {}

bool BudgetRepositoryImpl::save(const Budget& budget) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        INSERT INTO budgets 
        (id, tenant_id, workspace_id, name, category_id, amount, spent, period, 
         start_date, end_date, is_recurring, alert_percentage, status, created_by, 
         created_at, updated_at)
        VALUES ($1, $2, NULLIF($3,''), $4, NULLIF($5,''), $6, $7, $8, $9, $10, $11, $12, $13, $14,
                CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
    )";
    
    std::vector<std::string> params = {
        budget.getId(),
        budget.getTenantId(),
        budget.getWorkspaceId(),
        budget.getName(),
        budget.getCategoryId().value_or(""),
        std::to_string(budget.getAmount()),
        std::to_string(budget.getSpent()),
        budget.getPeriod().toString(),
        budget.getStartDate(),
        budget.getEndDate(),
        budget.isRecurring() ? "true" : "false",
        std::to_string(budget.getAlertPercentage()),
        budget.getStatus(),
        budget.getCreatedBy()
    };
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        Utils::Logger::error("[BudgetRepository::save] Failed to save budget: " + budget.getId());
        return false;
    }
    
    Utils::Logger::info("[BudgetRepository::save] Budget saved successfully: " + budget.getId());
    return true;
}

bool BudgetRepositoryImpl::update(const Budget& budget) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE budgets 
        SET workspace_id = NULLIF($3,''),
            name = $4,
            category_id = NULLIF($5,''),
            amount = $6,
            spent = $7,
            period = $8,
            start_date = $9,
            end_date = $10,
            is_recurring = $11,
            alert_percentage = $12,
            status = $13,
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $1 AND tenant_id = $2
    )";
    
    std::vector<std::string> params = {
        budget.getId(),
        budget.getTenantId(),
        budget.getWorkspaceId(),
        budget.getName(),
        budget.getCategoryId().value_or(""),
        std::to_string(budget.getAmount()),
        std::to_string(budget.getSpent()),
        budget.getPeriod().toString(),
        budget.getStartDate(),
        budget.getEndDate(),
        budget.isRecurring() ? "true" : "false",
        std::to_string(budget.getAlertPercentage()),
        budget.getStatus()
    };
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        Utils::Logger::error("[BudgetRepository::update] Failed to update budget: " + budget.getId());
        return false;
    }
    
    Utils::Logger::info("[BudgetRepository::update] Budget updated successfully: " + budget.getId());
    return true;
}

bool BudgetRepositoryImpl::remove(const std::string& id) {
    auto conn = pool_->acquire();
    
    std::string query = "DELETE FROM budgets WHERE id = $1";
    std::vector<std::string> params = {id};
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        Utils::Logger::error("[BudgetRepository::remove] Failed to remove budget: " + id);
        return false;
    }
    
    Utils::Logger::info("[BudgetRepository::remove] Budget removed successfully: " + id);
    return true;
}

std::optional<Budget> BudgetRepositoryImpl::findById(const std::string& id, const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM budgets WHERE id = $1 AND tenant_id = $2";
    std::vector<std::string> params = {id, tenantId};
    
    auto result = conn->executeParams(query, params);
    
    if (result.rowCount() == 0) {
        Utils::Logger::debug("[BudgetRepository::findById] Budget not found: " + id);
        return std::nullopt;
    }
    
    return mapToBudget(result, 0);
}

std::vector<Budget> BudgetRepositoryImpl::findByTenant(const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM budgets WHERE tenant_id = $1 ORDER BY start_date DESC";
    std::vector<std::string> params = {tenantId};
    
    auto result = conn->executeParams(query, params);
    
    std::vector<Budget> budgets;
    for (int i = 0; i < result.rowCount(); i++) {
        budgets.push_back(mapToBudget(result, i));
    }
    
    Utils::Logger::debug("[BudgetRepository::findByTenant] Found " + std::to_string(budgets.size()) + " budgets for tenant: " + tenantId);
    return budgets;
}

std::vector<Budget> BudgetRepositoryImpl::findByCategory(const std::string& categoryId, const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM budgets WHERE category_id = $1 AND tenant_id = $2 ORDER BY start_date DESC";
    std::vector<std::string> params = {categoryId, tenantId};
    
    auto result = conn->executeParams(query, params);
    
    std::vector<Budget> budgets;
    for (int i = 0; i < result.rowCount(); i++) {
        budgets.push_back(mapToBudget(result, i));
    }
    
    Utils::Logger::debug("[BudgetRepository::findByCategory] Found " + std::to_string(budgets.size()) + " budgets for category: " + categoryId);
    return budgets;
}

std::vector<Budget> BudgetRepositoryImpl::findByPeriod(const std::string& period, const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM budgets WHERE period = $1 AND tenant_id = $2 ORDER BY start_date DESC";
    std::vector<std::string> params = {period, tenantId};
    
    auto result = conn->executeParams(query, params);
    
    std::vector<Budget> budgets;
    for (int i = 0; i < result.rowCount(); i++) {
        budgets.push_back(mapToBudget(result, i));
    }
    
    Utils::Logger::debug("[BudgetRepository::findByPeriod] Found " + std::to_string(budgets.size()) + " budgets for period: " + period);
    return budgets;
}

std::vector<Budget> BudgetRepositoryImpl::findActive(const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM budgets WHERE status = 'active' AND tenant_id = $1 ORDER BY start_date DESC";
    std::vector<std::string> params = {tenantId};
    
    auto result = conn->executeParams(query, params);
    
    std::vector<Budget> budgets;
    for (int i = 0; i < result.rowCount(); i++) {
        budgets.push_back(mapToBudget(result, i));
    }
    
    Utils::Logger::debug("[BudgetRepository::findActive] Found " + std::to_string(budgets.size()) + " active budgets");
    return budgets;
}

std::vector<Budget> BudgetRepositoryImpl::findActiveForDate(const std::string& date, const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT * FROM budgets 
        WHERE tenant_id = $1 
          AND status = 'active'
          AND start_date <= $2::date 
          AND end_date >= $2::date
        ORDER BY start_date DESC
    )";
    
    std::vector<std::string> params = {tenantId, date};
    
    auto result = conn->executeParams(query, params);
    
    std::vector<Budget> budgets;
    for (int i = 0; i < result.rowCount(); i++) {
        budgets.push_back(mapToBudget(result, i));
    }
    
    Utils::Logger::debug("[BudgetRepository::findActiveForDate] Found " + std::to_string(budgets.size()) + " active budgets for date: " + date);
    return budgets;
}

bool BudgetRepositoryImpl::updateSpent(const std::string& id, double newSpent) {
    auto conn = pool_->acquire();
    
    // Get current budget to update status
    auto budgetOpt = findById(id, ""); // We'll check by ID only for this internal operation
    if (!budgetOpt.has_value()) {
        Utils::Logger::error("[BudgetRepository::updateSpent] Budget not found: " + id);
        return false;
    }
    
    auto budget = budgetOpt.value();
    double amount = budget.getAmount();
    
    // Determine new status based on spent amount
    std::string newStatus = "active";
    if (newSpent >= amount) {
        newStatus = "completed";
    }
    if (newSpent > amount) {
        newStatus = "exceeded";
    }
    
    std::string query = R"(
        UPDATE budgets 
        SET spent = $2, 
            status = $3,
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $1
    )";
    
    std::vector<std::string> params = {id, std::to_string(newSpent), newStatus};
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        Utils::Logger::error("[BudgetRepository::updateSpent] Failed to update spent amount for budget: " + id);
        return false;
    }
    
    Utils::Logger::info("[BudgetRepository::updateSpent] Budget spent updated: " + id + " -> " + std::to_string(newSpent) + " (status: " + newStatus + ")");
    return true;
}

Budget BudgetRepositoryImpl::mapToBudget(const Core::Database::QueryResult& result, int row) {
    // id, tenant_id, workspace_id, name, category_id, amount, spent, period, start_date, end_date, 
    // is_recurring, alert_percentage, status, created_by, created_at, updated_at
    
    std::string id = result.getValue(row, 0);
    std::string tenantId = result.getValue(row, 1);
    std::string workspaceId = result.getValue(row, 2);
    std::string name = result.getValue(row, 3);
    std::string categoryId = result.getValue(row, 4);
    double amount = std::stod(result.getValue(row, 5));
    double spent = std::stod(result.getValue(row, 6));
    std::string periodStr = result.getValue(row, 7);
    std::string startDate = result.getValue(row, 8);
    std::string endDate = result.getValue(row, 9);
    bool isRecurring = result.getValue(row, 10) == "t" || result.getValue(row, 10) == "true" || result.getValue(row, 10) == "1";
    int alertPercentage = std::stoi(result.getValue(row, 11));
    std::string status = result.getValue(row, 12);
    std::string createdBy = result.getValue(row, 13);
    
    Budget budget(
        id,
        tenantId,
        name,
        amount,
        spent,
        BudgetPeriod(periodStr),
        startDate,
        endDate,
        isRecurring,
        alertPercentage,
        status,
        createdBy
    );
    
    budget.setWorkspaceId(workspaceId);
    
    if (!categoryId.empty()) {
        budget.setCategoryId(categoryId);
    }
    
    return budget;
}

} // namespace Finance

