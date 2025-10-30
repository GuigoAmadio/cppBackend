#pragma once
#include <memory>
#include <map>
#include <vector>
#include "../../repositories/TransactionRepository.hpp"
#include "../../repositories/BudgetRepository.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {
namespace Utils = Core::Utils;

// ===== 1. CASH FLOW REPORT =====
struct CashFlowReport {
    double totalIncome;
    double totalExpense;
    double netCashFlow;
    std::string startDate;
    std::string endDate;
    std::string currency;
};

class GetCashFlowUseCase {
public:
    explicit GetCashFlowUseCase(std::shared_ptr<TransactionRepository> transactionRepo)
        : transactionRepo_(transactionRepo) {}

    CashFlowReport execute(const std::string& tenantId, const std::string& startDate,
                          const std::string& endDate, const std::string& currency = "BRL") {
        Utils::Logger::info("[GetCashFlowUseCase] Generating cash flow report");

        if (tenantId.empty() || startDate.empty() || endDate.empty()) {
            throw std::invalid_argument("Tenant ID, start date, and end date are required");
        }

        auto cashFlow = transactionRepo_->getCashFlow(tenantId, startDate, endDate);

        CashFlowReport report;
        report.totalIncome = cashFlow["income"];
        report.totalExpense = cashFlow["expense"];
        report.netCashFlow = cashFlow["net"];
        report.startDate = startDate;
        report.endDate = endDate;
        report.currency = currency;

        Utils::Logger::info("[GetCashFlowUseCase] Report generated - Income: " + std::to_string(report.totalIncome) +
                           ", Expense: " + std::to_string(report.totalExpense) +
                           ", Net: " + std::to_string(report.netCashFlow));

        return report;
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
};

// ===== 2. INCOME VS EXPENSE REPORT =====
struct IncomeVsExpenseReport {
    double totalIncome;
    double totalExpense;
    double difference;
    double percentageIncome;
    double percentageExpense;
    std::string startDate;
    std::string endDate;
};

class GetIncomeVsExpenseUseCase {
public:
    explicit GetIncomeVsExpenseUseCase(std::shared_ptr<TransactionRepository> transactionRepo)
        : transactionRepo_(transactionRepo) {}

    IncomeVsExpenseReport execute(const std::string& tenantId, const std::string& startDate, const std::string& endDate) {
        Utils::Logger::info("[GetIncomeVsExpenseUseCase] Generating income vs expense report");

        auto cashFlow = transactionRepo_->getCashFlow(tenantId, startDate, endDate);

        IncomeVsExpenseReport report;
        report.totalIncome = cashFlow["income"];
        report.totalExpense = cashFlow["expense"];
        report.difference = report.totalIncome - report.totalExpense;
        report.startDate = startDate;
        report.endDate = endDate;

        double total = report.totalIncome + report.totalExpense;
        if (total > 0) {
            report.percentageIncome = (report.totalIncome / total) * 100.0;
            report.percentageExpense = (report.totalExpense / total) * 100.0;
        } else {
            report.percentageIncome = 0.0;
            report.percentageExpense = 0.0;
        }

        return report;
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
};

// ===== 3. CATEGORY REPORT =====
struct CategoryReportItem {
    std::string categoryName;
    double total;
    double percentage;
};

class GetCategoryReportUseCase {
public:
    explicit GetCategoryReportUseCase(std::shared_ptr<TransactionRepository> transactionRepo)
        : transactionRepo_(transactionRepo) {}

    std::vector<CategoryReportItem> execute(const std::string& tenantId, const std::string& type,
                                           const std::string& startDate, const std::string& endDate) {
        Utils::Logger::info("[GetCategoryReportUseCase] Generating category report for type: " + type);

        if (type != "income" && type != "expense") {
            throw std::invalid_argument("Type must be 'income' or 'expense'");
        }

        auto totals = transactionRepo_->getTotalsByCategory(type, tenantId, startDate, endDate);

        // Calculate grand total
        double grandTotal = 0.0;
        for (const auto& pair : totals) {
            grandTotal += pair.second;
        }

        // Build report items with percentages
        std::vector<CategoryReportItem> items;
        for (const auto& pair : totals) {
            CategoryReportItem item;
            item.categoryName = pair.first;
            item.total = pair.second;
            item.percentage = (grandTotal > 0) ? (pair.second / grandTotal) * 100.0 : 0.0;
            items.push_back(item);
        }

        Utils::Logger::info("[GetCategoryReportUseCase] Report generated with " + std::to_string(items.size()) + " categories");
        return items;
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
};

// ===== 4. PROFIT & LOSS (P&L) REPORT =====
struct ProfitLossReport {
    // Revenue
    double totalRevenue;
    std::map<std::string, double> revenueByCategory;
    
    // Expenses
    double totalExpenses;
    std::map<std::string, double> expensesByCategory;
    
    // Profit/Loss
    double grossProfit;
    double netProfit;
    double profitMargin;  // percentage
    
    std::string startDate;
    std::string endDate;
};

class GetProfitLossUseCase {
public:
    explicit GetProfitLossUseCase(std::shared_ptr<TransactionRepository> transactionRepo)
        : transactionRepo_(transactionRepo) {}

    ProfitLossReport execute(const std::string& tenantId, const std::string& startDate, const std::string& endDate) {
        Utils::Logger::info("[GetProfitLossUseCase] Generating P&L report");

        ProfitLossReport report;
        report.startDate = startDate;
        report.endDate = endDate;

        // Get revenue
        report.totalRevenue = transactionRepo_->getTotalByType("income", tenantId, startDate, endDate);
        report.revenueByCategory = transactionRepo_->getTotalsByCategory("income", tenantId, startDate, endDate);

        // Get expenses
        report.totalExpenses = transactionRepo_->getTotalByType("expense", tenantId, startDate, endDate);
        report.expensesByCategory = transactionRepo_->getTotalsByCategory("expense", tenantId, startDate, endDate);

        // Calculate profit
        report.grossProfit = report.totalRevenue - report.totalExpenses;
        report.netProfit = report.grossProfit;  // In a real system, you'd subtract taxes, etc.
        
        report.profitMargin = (report.totalRevenue > 0) ?
                             (report.netProfit / report.totalRevenue) * 100.0 : 0.0;

        Utils::Logger::info("[GetProfitLossUseCase] P&L Report - Revenue: " + std::to_string(report.totalRevenue) +
                           ", Expenses: " + std::to_string(report.totalExpenses) +
                           ", Net Profit: " + std::to_string(report.netProfit) +
                           ", Margin: " + std::to_string(report.profitMargin) + "%");

        return report;
    }

private:
    std::shared_ptr<TransactionRepository> transactionRepo_;
};

// ===== 5. BUDGET PERFORMANCE REPORT =====
struct BudgetPerformanceItem {
    std::string budgetId;
    std::string budgetName;
    std::string categoryName;
    double budgetAmount;
    double spent;
    double remaining;
    double percentageUsed;
    std::string status;  // active, completed, exceeded
    bool shouldAlert;
};

class GetBudgetPerformanceUseCase {
public:
    explicit GetBudgetPerformanceUseCase(
        std::shared_ptr<BudgetRepository> budgetRepo,
        std::shared_ptr<TransactionRepository> transactionRepo
    ) : budgetRepo_(budgetRepo), transactionRepo_(transactionRepo) {}

    std::vector<BudgetPerformanceItem> execute(const std::string& tenantId, const std::string& date = "") {
        Utils::Logger::info("[GetBudgetPerformanceUseCase] Generating budget performance report");

        std::vector<Budget> budgets;
        
        if (!date.empty()) {
            budgets = budgetRepo_->findActiveForDate(date, tenantId);
        } else {
            budgets = budgetRepo_->findActive(tenantId);
        }

        std::vector<BudgetPerformanceItem> items;
        
        for (const auto& budget : budgets) {
            BudgetPerformanceItem item;
            item.budgetId = budget.getId();
            item.budgetName = budget.getName();
            item.categoryName = budget.getCategoryId().value_or("All categories");
            item.budgetAmount = budget.getAmount();
            item.spent = budget.getSpent();
            item.remaining = budget.getRemaining();
            item.percentageUsed = budget.getPercentageSpent();
            item.status = budget.getStatus();
            item.shouldAlert = budget.shouldAlert();
            
            items.push_back(item);
        }

        Utils::Logger::info("[GetBudgetPerformanceUseCase] Report generated with " + std::to_string(items.size()) + " budgets");
        return items;
    }

private:
    std::shared_ptr<BudgetRepository> budgetRepo_;
    std::shared_ptr<TransactionRepository> transactionRepo_;
};

} // namespace Finance

