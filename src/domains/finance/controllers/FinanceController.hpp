#pragma once
#include <memory>
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"

// Repositories
#include "../repositories/AccountRepository.hpp"
#include "../repositories/TransactionRepository.hpp"
#include "../repositories/CategoryRepository.hpp"
#include "../repositories/BudgetRepository.hpp"

// Use Cases - Account
#include "../use_cases/account/CreateAccountUseCase.hpp"
#include "../use_cases/account/GetAccountUseCase.hpp"
#include "../use_cases/account/ListAccountsUseCase.hpp"
#include "../use_cases/account/UpdateAccountUseCase.hpp"
#include "../use_cases/account/DeleteAccountUseCase.hpp"
#include "../use_cases/account/GetAccountBalanceUseCase.hpp"

// Use Cases - Transaction
#include "../use_cases/transaction/CreateTransactionUseCase.hpp"
#include "../use_cases/transaction/CreateTransferUseCase.hpp"
#include "../use_cases/transaction/GetTransactionUseCase.hpp"
#include "../use_cases/transaction/ListTransactionsUseCase.hpp"
#include "../use_cases/transaction/UpdateTransactionUseCase.hpp"
#include "../use_cases/transaction/DeleteTransactionUseCase.hpp"
#include "../use_cases/transaction/CompleteTransactionUseCase.hpp"

// Use Cases - Category
#include "../use_cases/category/CategoryUseCases.hpp"

// Use Cases - Budget
#include "../use_cases/budget/BudgetUseCases.hpp"

// Use Cases - Reports
#include "../use_cases/report/ReportUseCases.hpp"

namespace Finance {

class FinanceController {
public:
    FinanceController(
        std::shared_ptr<AccountRepository> accountRepo,
        std::shared_ptr<TransactionRepository> transactionRepo,
        std::shared_ptr<CategoryRepository> categoryRepo,
        std::shared_ptr<BudgetRepository> budgetRepo
    );

    // ========== ACCOUNT ENDPOINTS (6) ==========
    Core::Http::Response createAccount(const Core::Http::Request& req);
    Core::Http::Response getAccount(const Core::Http::Request& req);
    Core::Http::Response listAccounts(const Core::Http::Request& req);
    Core::Http::Response updateAccount(const Core::Http::Request& req);
    Core::Http::Response deleteAccount(const Core::Http::Request& req);
    Core::Http::Response getAccountBalance(const Core::Http::Request& req);

    // ========== TRANSACTION ENDPOINTS (8) ==========
    Core::Http::Response createTransaction(const Core::Http::Request& req);
    Core::Http::Response createTransfer(const Core::Http::Request& req);
    Core::Http::Response getTransaction(const Core::Http::Request& req);
    Core::Http::Response listTransactions(const Core::Http::Request& req);
    Core::Http::Response updateTransaction(const Core::Http::Request& req);
    Core::Http::Response deleteTransaction(const Core::Http::Request& req);
    Core::Http::Response completeTransaction(const Core::Http::Request& req);
    Core::Http::Response cancelTransaction(const Core::Http::Request& req);

    // ========== CATEGORY ENDPOINTS (5) ==========
    Core::Http::Response createCategory(const Core::Http::Request& req);
    Core::Http::Response getCategory(const Core::Http::Request& req);
    Core::Http::Response listCategories(const Core::Http::Request& req);
    Core::Http::Response updateCategory(const Core::Http::Request& req);
    Core::Http::Response deleteCategory(const Core::Http::Request& req);

    // ========== BUDGET ENDPOINTS (5) ==========
    Core::Http::Response createBudget(const Core::Http::Request& req);
    Core::Http::Response getBudget(const Core::Http::Request& req);
    Core::Http::Response listBudgets(const Core::Http::Request& req);
    Core::Http::Response updateBudget(const Core::Http::Request& req);
    Core::Http::Response deleteBudget(const Core::Http::Request& req);

    // ========== REPORT ENDPOINTS (5) ==========
    Core::Http::Response getCashFlow(const Core::Http::Request& req);
    Core::Http::Response getIncomeVsExpense(const Core::Http::Request& req);
    Core::Http::Response getCategoryReport(const Core::Http::Request& req);
    Core::Http::Response getProfitLoss(const Core::Http::Request& req);
    Core::Http::Response getBudgetPerformance(const Core::Http::Request& req);

private:
    // Repositories
    std::shared_ptr<AccountRepository> accountRepo_;
    std::shared_ptr<TransactionRepository> transactionRepo_;
    std::shared_ptr<CategoryRepository> categoryRepo_;
    std::shared_ptr<BudgetRepository> budgetRepo_;

    // Use Cases - Account
    std::shared_ptr<CreateAccountUseCase> createAccountUseCase_;
    std::shared_ptr<GetAccountUseCase> getAccountUseCase_;
    std::shared_ptr<ListAccountsUseCase> listAccountsUseCase_;
    std::shared_ptr<UpdateAccountUseCase> updateAccountUseCase_;
    std::shared_ptr<DeleteAccountUseCase> deleteAccountUseCase_;
    std::shared_ptr<GetAccountBalanceUseCase> getAccountBalanceUseCase_;

    // Use Cases - Transaction
    std::shared_ptr<CreateTransactionUseCase> createTransactionUseCase_;
    std::shared_ptr<CreateTransferUseCase> createTransferUseCase_;
    std::shared_ptr<GetTransactionUseCase> getTransactionUseCase_;
    std::shared_ptr<ListTransactionsUseCase> listTransactionsUseCase_;
    std::shared_ptr<UpdateTransactionUseCase> updateTransactionUseCase_;
    std::shared_ptr<DeleteTransactionUseCase> deleteTransactionUseCase_;
    std::shared_ptr<CompleteTransactionUseCase> completeTransactionUseCase_;
    std::shared_ptr<CancelTransactionUseCase> cancelTransactionUseCase_;

    // Use Cases - Category
    std::shared_ptr<CreateCategoryUseCase> createCategoryUseCase_;
    std::shared_ptr<GetCategoryUseCase> getCategoryUseCase_;
    std::shared_ptr<ListCategoriesUseCase> listCategoriesUseCase_;
    std::shared_ptr<UpdateCategoryUseCase> updateCategoryUseCase_;
    std::shared_ptr<DeleteCategoryUseCase> deleteCategoryUseCase_;

    // Use Cases - Budget
    std::shared_ptr<CreateBudgetUseCase> createBudgetUseCase_;
    std::shared_ptr<GetBudgetUseCase> getBudgetUseCase_;
    std::shared_ptr<ListBudgetsUseCase> listBudgetsUseCase_;
    std::shared_ptr<UpdateBudgetUseCase> updateBudgetUseCase_;
    std::shared_ptr<DeleteBudgetUseCase> deleteBudgetUseCase_;

    // Use Cases - Reports
    std::shared_ptr<GetCashFlowUseCase> getCashFlowUseCase_;
    std::shared_ptr<GetIncomeVsExpenseUseCase> getIncomeVsExpenseUseCase_;
    std::shared_ptr<GetCategoryReportUseCase> getCategoryReportUseCase_;
    std::shared_ptr<GetProfitLossUseCase> getProfitLossUseCase_;
    std::shared_ptr<GetBudgetPerformanceUseCase> getBudgetPerformanceUseCase_;

    // Helper methods
    std::string getTenantId(const Core::Http::Request& req);
    std::string getUserId(const Core::Http::Request& req);
    std::string getWorkspaceId(const Core::Http::Request& req);
};

} // namespace Finance

