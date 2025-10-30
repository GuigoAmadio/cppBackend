#pragma once
#include <memory>
#include <vector>
#include <optional>
#include "../../repositories/BudgetRepository.hpp"
#include "../../entities/Budget.hpp"
#include "../../value_objects/BudgetPeriod.hpp"
#include "../../../../core/utils/Logger.hpp"
#include "../../../../core/utils/IdGenerator.hpp"

namespace Finance {
namespace Utils = Core::Utils;

// ===== CREATE BUDGET =====
struct CreateBudgetDTO {
    std::string tenantId;
    std::string workspaceId;
    std::string name;
    std::string categoryId;
    double amount;
    std::string period;  // daily, weekly, monthly, quarterly, yearly, custom
    std::string startDate;
    std::string endDate;
    bool isRecurring;
    int alertPercentage;
    std::string createdBy;
};

class CreateBudgetUseCase {
public:
    explicit CreateBudgetUseCase(std::shared_ptr<BudgetRepository> budgetRepo)
        : budgetRepo_(budgetRepo) {}

    std::string execute(const CreateBudgetDTO& dto) {
        if (dto.tenantId.empty() || dto.name.empty() || dto.amount <= 0 || 
            dto.period.empty() || dto.startDate.empty() || dto.endDate.empty() || dto.createdBy.empty()) {
            throw std::invalid_argument("Missing required fields");
        }

        std::string id = Core::Utils::IdGenerator::generate("bdg");
        
        Budget budget(id, dto.tenantId, dto.name, dto.amount, 0.0, BudgetPeriod(dto.period),
                     dto.startDate, dto.endDate, dto.isRecurring, dto.alertPercentage, "active", dto.createdBy);
        
        if (!dto.workspaceId.empty()) budget.setWorkspaceId(dto.workspaceId);
        if (!dto.categoryId.empty()) budget.setCategoryId(dto.categoryId);

        if (!budgetRepo_->save(budget)) throw std::runtime_error("Failed to save budget");
        return id;
    }

private:
    std::shared_ptr<BudgetRepository> budgetRepo_;
};

// ===== GET BUDGET =====
class GetBudgetUseCase {
public:
    explicit GetBudgetUseCase(std::shared_ptr<BudgetRepository> budgetRepo)
        : budgetRepo_(budgetRepo) {}

    Budget execute(const std::string& budgetId, const std::string& tenantId) {
        auto budgetOpt = budgetRepo_->findById(budgetId, tenantId);
        if (!budgetOpt.has_value()) throw std::runtime_error("Budget not found");
        return budgetOpt.value();
    }

private:
    std::shared_ptr<BudgetRepository> budgetRepo_;
};

// ===== LIST BUDGETS =====
class ListBudgetsUseCase {
public:
    explicit ListBudgetsUseCase(std::shared_ptr<BudgetRepository> budgetRepo)
        : budgetRepo_(budgetRepo) {}

    std::vector<Budget> execute(const std::string& tenantId, const std::string& categoryId = "",
                                const std::string& period = "", bool activeOnly = false) {
        if (!categoryId.empty()) {
            return budgetRepo_->findByCategory(categoryId, tenantId);
        } else if (!period.empty()) {
            return budgetRepo_->findByPeriod(period, tenantId);
        } else if (activeOnly) {
            return budgetRepo_->findActive(tenantId);
        } else {
            return budgetRepo_->findByTenant(tenantId);
        }
    }

private:
    std::shared_ptr<BudgetRepository> budgetRepo_;
};

// ===== UPDATE BUDGET =====
struct UpdateBudgetDTO {
    std::string budgetId;
    std::string tenantId;
    std::optional<std::string> name;
    std::optional<double> amount;
    std::optional<int> alertPercentage;
};

class UpdateBudgetUseCase {
public:
    explicit UpdateBudgetUseCase(std::shared_ptr<BudgetRepository> budgetRepo)
        : budgetRepo_(budgetRepo) {}

    bool execute(const UpdateBudgetDTO& dto) {
        auto budgetOpt = budgetRepo_->findById(dto.budgetId, dto.tenantId);
        if (!budgetOpt.has_value()) throw std::runtime_error("Budget not found");

        Budget budget = budgetOpt.value();
        
        if (dto.name.has_value()) budget.setName(*dto.name);
        if (dto.amount.has_value()) budget.setAmount(*dto.amount);
        if (dto.alertPercentage.has_value()) budget.setAlertPercentage(*dto.alertPercentage);

        return budgetRepo_->update(budget);
    }

private:
    std::shared_ptr<BudgetRepository> budgetRepo_;
};

// ===== DELETE BUDGET =====
class DeleteBudgetUseCase {
public:
    explicit DeleteBudgetUseCase(std::shared_ptr<BudgetRepository> budgetRepo)
        : budgetRepo_(budgetRepo) {}

    bool execute(const std::string& budgetId, const std::string& tenantId) {
        auto budgetOpt = budgetRepo_->findById(budgetId, tenantId);
        if (!budgetOpt.has_value()) throw std::runtime_error("Budget not found");

        return budgetRepo_->remove(budgetId);
    }

private:
    std::shared_ptr<BudgetRepository> budgetRepo_;
};

} // namespace Finance

