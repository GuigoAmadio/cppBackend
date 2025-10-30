# 🔧 FINANCE MODULE - INTEGRATION GUIDE

## ✅ **WHAT'S BEEN CREATED**

### **1. Value Objects (5)**

- `src/domains/finance/value_objects/AccountType.hpp`
- `src/domains/finance/value_objects/TransactionType.hpp`
- `src/domains/finance/value_objects/TransactionStatus.hpp`
- `src/domains/finance/value_objects/BudgetPeriod.hpp`
- `src/domains/finance/value_objects/ReconciliationStatus.hpp`

### **2. Entities (5)**

- `src/domains/finance/entities/Account.hpp`
- `src/domains/finance/entities/Transaction.hpp`
- `src/domains/finance/entities/Category.hpp`
- `src/domains/finance/entities/Budget.hpp`
- `src/domains/finance/entities/Reconciliation.hpp`

### **3. Repositories (5 interfaces + 5 implementations)**

- **Interfaces:**

  - `src/domains/finance/repositories/AccountRepository.hpp`
  - `src/domains/finance/repositories/TransactionRepository.hpp`
  - `src/domains/finance/repositories/CategoryRepository.hpp`
  - `src/domains/finance/repositories/BudgetRepository.hpp`
  - `src/domains/finance/repositories/ReconciliationRepository.hpp`

- **Implementations:**
  - `src/domains/finance/repositories/impl/AccountRepositoryImpl.hpp/cpp`
  - `src/domains/finance/repositories/impl/TransactionRepositoryImpl.hpp/cpp`
  - `src/domains/finance/repositories/impl/CategoryRepositoryImpl.hpp/cpp`
  - `src/domains/finance/repositories/impl/BudgetRepositoryImpl.hpp/cpp`
  - `src/domains/finance/repositories/impl/ReconciliationRepositoryImpl.hpp/cpp`

### **4. Use Cases (25 - consolidados em 5 arquivos)**

- `src/domains/finance/use_cases/account/` (6 use cases)
- `src/domains/finance/use_cases/transaction/` (8 use cases)
- `src/domains/finance/use_cases/category/CategoryUseCases.hpp` (5 use cases)
- `src/domains/finance/use_cases/budget/BudgetUseCases.hpp` (5 use cases)
- `src/domains/finance/use_cases/report/ReportUseCases.hpp` (5 use cases)

### **5. Controller**

- `src/domains/finance/controllers/FinanceController.hpp/cpp` (29 endpoints)

---

## 📝 **STEP 1: UPDATE CMakeLists.txt**

Add the following source files to `CMakeLists.txt` under the `add_executable(cppBackend ...)` section:

```cmake
# Finance Module
src/domains/finance/repositories/impl/AccountRepositoryImpl.cpp
src/domains/finance/repositories/impl/TransactionRepositoryImpl.cpp
src/domains/finance/repositories/impl/CategoryRepositoryImpl.cpp
src/domains/finance/repositories/impl/BudgetRepositoryImpl.cpp
src/domains/finance/repositories/impl/ReconciliationRepositoryImpl.cpp
src/domains/finance/controllers/FinanceController.cpp
```

**Total:** 6 new `.cpp` files

---

## 📝 **STEP 2: UPDATE main_new.cpp**

### **2.1. Add Includes (after existing includes)**

```cpp
// Finance Module
#include "src/domains/finance/repositories/impl/AccountRepositoryImpl.hpp"
#include "src/domains/finance/repositories/impl/TransactionRepositoryImpl.hpp"
#include "src/domains/finance/repositories/impl/CategoryRepositoryImpl.hpp"
#include "src/domains/finance/repositories/impl/BudgetRepositoryImpl.hpp"
#include "src/domains/finance/repositories/impl/ReconciliationRepositoryImpl.hpp"
#include "src/domains/finance/controllers/FinanceController.hpp"
```

### **2.2. Instantiate Repositories (inside main(), after existing repos)**

```cpp
// ========== Finance Repositories ==========
Utils::Logger::info("[MAIN] Creating Finance repositories...");
auto financeAccountRepo = std::make_shared<Finance::AccountRepositoryImpl>(dbPool);
auto financeTransactionRepo = std::make_shared<Finance::TransactionRepositoryImpl>(dbPool);
auto financeCategoryRepo = std::make_shared<Finance::CategoryRepositoryImpl>(dbPool);
auto financeBudgetRepo = std::make_shared<Finance::BudgetRepositoryImpl>(dbPool);
auto financeReconciliationRepo = std::make_shared<Finance::ReconciliationRepositoryImpl>(dbPool);
```

### **2.3. Instantiate Controller (after existing controllers)**

```cpp
// ========== Finance Controller ==========
Utils::Logger::info("[MAIN] Creating Finance controller...");
auto financeController = std::make_shared<Finance::FinanceController>(
    financeAccountRepo,
    financeTransactionRepo,
    financeCategoryRepo,
    financeBudgetRepo
);
```

### **2.4. Register Routes (after existing routes)**

```cpp
// ========== FINANCE ROUTES (29 endpoints) ==========
Utils::Logger::info("[MAIN] Registering Finance routes...");

// Account routes (6)
app.post("/api/finance/accounts", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->createAccount(req);
});
app.get("/api/finance/accounts/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->getAccount(req);
});
app.get("/api/finance/accounts", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->listAccounts(req);
});
app.put("/api/finance/accounts/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->updateAccount(req);
});
app.del("/api/finance/accounts/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->deleteAccount(req);
});
app.get("/api/finance/accounts/:id/balance", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->getAccountBalance(req);
});

// Transaction routes (8)
app.post("/api/finance/transactions", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->createTransaction(req);
});
app.post("/api/finance/transactions/transfer", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->createTransfer(req);
});
app.get("/api/finance/transactions/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->getTransaction(req);
});
app.get("/api/finance/transactions", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->listTransactions(req);
});
app.put("/api/finance/transactions/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->updateTransaction(req);
});
app.del("/api/finance/transactions/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->deleteTransaction(req);
});
app.post("/api/finance/transactions/:id/complete", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->completeTransaction(req);
});
app.post("/api/finance/transactions/:id/cancel", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->cancelTransaction(req);
});

// Category routes (5)
app.post("/api/finance/categories", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->createCategory(req);
});
app.get("/api/finance/categories/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->getCategory(req);
});
app.get("/api/finance/categories", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->listCategories(req);
});
app.put("/api/finance/categories/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->updateCategory(req);
});
app.del("/api/finance/categories/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->deleteCategory(req);
});

// Budget routes (5)
app.post("/api/finance/budgets", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->createBudget(req);
});
app.get("/api/finance/budgets/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->getBudget(req);
});
app.get("/api/finance/budgets", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->listBudgets(req);
});
app.put("/api/finance/budgets/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->updateBudget(req);
});
app.del("/api/finance/budgets/:id", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->deleteBudget(req);
});

// Report routes (5)
app.get("/api/finance/reports/cashflow", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->getCashFlow(req);
});
app.get("/api/finance/reports/income-vs-expense", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->getIncomeVsExpense(req);
});
app.get("/api/finance/reports/by-category", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->getCategoryReport(req);
});
app.get("/api/finance/reports/profit-loss", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->getProfitLoss(req);
});
app.get("/api/finance/reports/budget-performance", authMiddleware, [financeController](const Core::Http::Request& req) {
    return financeController->getBudgetPerformance(req);
});

Utils::Logger::info("[MAIN] Finance module registered: 29 routes");
```

---

## 🎯 **SUMMARY**

### **Files to Update:**

1. ✅ `CMakeLists.txt` - Add 6 `.cpp` files
2. ✅ `main_new.cpp` - Add includes, repos, controller, 29 routes

### **Total Integration:**

- **6 includes**
- **5 repositories**
- **1 controller**
- **29 routes**

---

## 🚀 **NEXT STEPS AFTER INTEGRATION**

1. **Compile:**

   ```powershell
   cd C:\Users\Guillermo\Desktop\cppBackend\build
   cmake --build . --config Release
   ```

2. **Run server:**

   ```powershell
   .\Release\cppBackend.exe
   ```

3. **Test endpoints** (PowerShell test script will be created separately)

---

**Status:** Ready for integration!  
**Estimated compile time:** 3-5 minutes  
**Module completion:** ~95% (only testing remains)
