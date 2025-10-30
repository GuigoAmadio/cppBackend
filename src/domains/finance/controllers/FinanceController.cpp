#include "FinanceController.hpp"
#include "../../../core/utils/Logger.hpp"
#include "../../../core/json/Json.hpp"
#include <sstream>

namespace Finance {

namespace Utils = Core::Utils;

FinanceController::FinanceController(
    std::shared_ptr<AccountRepository> accountRepo,
    std::shared_ptr<TransactionRepository> transactionRepo,
    std::shared_ptr<CategoryRepository> categoryRepo,
    std::shared_ptr<BudgetRepository> budgetRepo
) : accountRepo_(accountRepo),
    transactionRepo_(transactionRepo),
    categoryRepo_(categoryRepo),
    budgetRepo_(budgetRepo) {
    
    // Initialize Account Use Cases
    createAccountUseCase_ = std::make_shared<CreateAccountUseCase>(accountRepo_);
    getAccountUseCase_ = std::make_shared<GetAccountUseCase>(accountRepo_);
    listAccountsUseCase_ = std::make_shared<ListAccountsUseCase>(accountRepo_);
    updateAccountUseCase_ = std::make_shared<UpdateAccountUseCase>(accountRepo_);
    deleteAccountUseCase_ = std::make_shared<DeleteAccountUseCase>(accountRepo_, transactionRepo_);
    getAccountBalanceUseCase_ = std::make_shared<GetAccountBalanceUseCase>(accountRepo_);

    // Initialize Transaction Use Cases
    createTransactionUseCase_ = std::make_shared<CreateTransactionUseCase>(transactionRepo_, accountRepo_, budgetRepo_);
    createTransferUseCase_ = std::make_shared<CreateTransferUseCase>(transactionRepo_, accountRepo_);
    getTransactionUseCase_ = std::make_shared<GetTransactionUseCase>(transactionRepo_);
    listTransactionsUseCase_ = std::make_shared<ListTransactionsUseCase>(transactionRepo_);
    updateTransactionUseCase_ = std::make_shared<UpdateTransactionUseCase>(transactionRepo_);
    deleteTransactionUseCase_ = std::make_shared<DeleteTransactionUseCase>(transactionRepo_, accountRepo_);
    completeTransactionUseCase_ = std::make_shared<CompleteTransactionUseCase>(transactionRepo_, accountRepo_);
    cancelTransactionUseCase_ = std::make_shared<CancelTransactionUseCase>(transactionRepo_, accountRepo_);

    // Initialize Category Use Cases
    createCategoryUseCase_ = std::make_shared<CreateCategoryUseCase>(categoryRepo_);
    getCategoryUseCase_ = std::make_shared<GetCategoryUseCase>(categoryRepo_);
    listCategoriesUseCase_ = std::make_shared<ListCategoriesUseCase>(categoryRepo_);
    updateCategoryUseCase_ = std::make_shared<UpdateCategoryUseCase>(categoryRepo_);
    deleteCategoryUseCase_ = std::make_shared<DeleteCategoryUseCase>(categoryRepo_);

    // Initialize Budget Use Cases
    createBudgetUseCase_ = std::make_shared<CreateBudgetUseCase>(budgetRepo_);
    getBudgetUseCase_ = std::make_shared<GetBudgetUseCase>(budgetRepo_);
    listBudgetsUseCase_ = std::make_shared<ListBudgetsUseCase>(budgetRepo_);
    updateBudgetUseCase_ = std::make_shared<UpdateBudgetUseCase>(budgetRepo_);
    deleteBudgetUseCase_ = std::make_shared<DeleteBudgetUseCase>(budgetRepo_);

    // Initialize Report Use Cases
    getCashFlowUseCase_ = std::make_shared<GetCashFlowUseCase>(transactionRepo_);
    getIncomeVsExpenseUseCase_ = std::make_shared<GetIncomeVsExpenseUseCase>(transactionRepo_);
    getCategoryReportUseCase_ = std::make_shared<GetCategoryReportUseCase>(transactionRepo_);
    getProfitLossUseCase_ = std::make_shared<GetProfitLossUseCase>(transactionRepo_);
    getBudgetPerformanceUseCase_ = std::make_shared<GetBudgetPerformanceUseCase>(budgetRepo_, transactionRepo_);

    Utils::Logger::info("[FinanceController] Initialized with all use cases");
}

// ========== HELPER METHODS ==========
std::string FinanceController::getTenantId(const Core::Http::Request& req) {
    return req.getCustomData("user_tenant_id");
}

std::string FinanceController::getUserId(const Core::Http::Request& req) {
    return req.getCustomData("user_id");
}

std::string FinanceController::getWorkspaceId(const Core::Http::Request& req) {
    return req.getQuery("workspace_id");
}

// ========== ACCOUNT ENDPOINTS ==========
Core::Http::Response FinanceController::createAccount(const Core::Http::Request& req) {
    Utils::Logger::info("[FinanceController::createAccount] CALLED!");
    try {
        Utils::Logger::info("[FinanceController::createAccount] Parsing JSON...");
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            auto json = Core::Json::makeObject();
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
        }
        auto& bodyObj = bodyJson->asObject();
        
        CreateAccountDTO dto;
        dto.tenantId = getTenantId(req);
        dto.workspaceId = getWorkspaceId(req); // Da query
        dto.name = bodyObj["name"]->asString();
        dto.type = bodyObj["type"]->asString();
        dto.currency = bodyObj["currency"]->asString();
        dto.initialBalance = bodyObj["initial_balance"]->asNumber();

        Utils::Logger::info("[FinanceController::createAccount] DTO populated: name=" + dto.name + ", type=" + dto.type + ", currency=" + dto.currency);
        Utils::Logger::info("[FinanceController::createAccount] Calling createAccountUseCase...");
        dto.bankName = bodyObj.count("bank_name") ? bodyObj["bank_name"]->asString() : "";
        dto.accountNumber = bodyObj.count("account_number") ? bodyObj["account_number"]->asString() : "";
        dto.createdBy = getUserId(req);

        Utils::Logger::info("[FinanceController::createAccount] DTO: name=" + dto.name + ", type=" + dto.type + ", currency=" + dto.currency);
        Utils::Logger::info("[FinanceController::createAccount] Calling createAccountUseCase->execute...");
        std::string accountId = createAccountUseCase_->execute(dto);
        Utils::Logger::info("[FinanceController::createAccount] Account created with ID: " + accountId);

        auto response = Core::Json::makeObject();
        response->asObject()["id"] = Core::Json::makeString(accountId);
        response->asObject()["message"] = Core::Json::makeString("Account created successfully");

        return Core::Http::Response(Core::Http::StatusCode::Created).json(*response);
    } catch (const std::exception& e) {
        Utils::Logger::info("[FinanceController::createAccount] ❌ EXCEPTION: " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::getAccount(const Core::Http::Request& req) {
    try {
        std::string accountId = req.getParam("id");
        std::string tenantId = getTenantId(req);

        auto account = getAccountUseCase_->execute(accountId, tenantId);

        auto response = Core::Json::makeObject();
        response->asObject()["id"] = Core::Json::makeString(account.getId());
        response->asObject()["tenant_id"] = Core::Json::makeString(account.getTenantId());
        response->asObject()["workspace_id"] = Core::Json::makeString(account.getWorkspaceId());
        response->asObject()["name"] = Core::Json::makeString(account.getName());
        response->asObject()["type"] = Core::Json::makeString(account.getType().toString());
        response->asObject()["currency"] = Core::Json::makeString(account.getCurrency());
        response->asObject()["balance"] = Core::Json::makeNumber(account.getBalance());
        response->asObject()["initial_balance"] = Core::Json::makeNumber(account.getInitialBalance());
        response->asObject()["is_active"] = Core::Json::makeBool(account.isActive());

        if (account.getBankName().has_value()) {
            response->asObject()["bank_name"] = Core::Json::makeString(*account.getBankName());
        }
        if (account.getAccountNumber().has_value()) {
            response->asObject()["account_number"] = Core::Json::makeString(*account.getAccountNumber());
        }

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::listAccounts(const Core::Http::Request& req) {
    try {
        std::string tenantId = getTenantId(req);
        std::string workspaceId = req.getQuery("workspace_id");
        std::string currency = req.getQuery("currency");
        bool activeOnly = req.getQuery("active") == "true";

        auto accounts = listAccountsUseCase_->execute(tenantId, workspaceId, currency, activeOnly);

        auto accountsArray = Core::Json::makeArray();
        for (const auto& account : accounts) {
            auto obj = Core::Json::makeObject();
            obj->asObject()["id"] = Core::Json::makeString(account.getId());
            obj->asObject()["name"] = Core::Json::makeString(account.getName());
            obj->asObject()["type"] = Core::Json::makeString(account.getType().toString());
            obj->asObject()["currency"] = Core::Json::makeString(account.getCurrency());
            obj->asObject()["balance"] = Core::Json::makeNumber(account.getBalance());
            obj->asObject()["is_active"] = Core::Json::makeBool(account.isActive());
            accountsArray->asArray().push_back(obj);
        }

        auto response = Core::Json::makeObject();
        response->asObject()["accounts"] = accountsArray;
        response->asObject()["total"] = Core::Json::makeNumber(accounts.size());

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::updateAccount(const Core::Http::Request& req) {
    try {
        std::string accountId = req.getParam("id");
        auto bodyJson = req.getJson();        if (!bodyJson || !bodyJson->isObject()) {            auto json = Core::Json::makeObject();            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);        }        auto& bodyObj = bodyJson->asObject();

        UpdateAccountDTO dto;
        dto.accountId = accountId;
        dto.tenantId = getTenantId(req);
        
        if (bodyObj.count("name")) dto.name = bodyObj["name"]->asString();
        if (bodyObj.count("bank_name")) dto.bankName = bodyObj["bank_name"]->asString();
        if (bodyObj.count("account_number")) dto.accountNumber = bodyObj["account_number"]->asString();
        if (bodyObj.count("is_active")) dto.isActive = bodyObj["is_active"]->asBool();

        updateAccountUseCase_->execute(dto);

        auto response = Core::Json::makeObject();
        response->asObject()["message"] = Core::Json::makeString("Account updated successfully");

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::deleteAccount(const Core::Http::Request& req) {
    try {
        std::string accountId = req.getParam("id");
        std::string tenantId = getTenantId(req);

        deleteAccountUseCase_->execute(accountId, tenantId);

        auto response = Core::Json::makeObject();
        response->asObject()["message"] = Core::Json::makeString("Account deleted successfully");

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::getAccountBalance(const Core::Http::Request& req) {
    try {
        std::string accountId = req.getParam("id");
        std::string tenantId = getTenantId(req);

        auto balance = getAccountBalanceUseCase_->execute(accountId, tenantId);

        auto response = Core::Json::makeObject();
        response->asObject()["account_id"] = Core::Json::makeString(balance.accountId);
        response->asObject()["name"] = Core::Json::makeString(balance.name);
        response->asObject()["type"] = Core::Json::makeString(balance.type);
        response->asObject()["currency"] = Core::Json::makeString(balance.currency);
        response->asObject()["balance"] = Core::Json::makeNumber(balance.balance);
        response->asObject()["initial_balance"] = Core::Json::makeNumber(balance.initialBalance);

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

// ========== TRANSACTION ENDPOINTS ==========
Core::Http::Response FinanceController::createTransaction(const Core::Http::Request& req) {
    try {
        auto bodyJson = req.getJson();        if (!bodyJson || !bodyJson->isObject()) {            auto json = Core::Json::makeObject();            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);        }        auto& bodyObj = bodyJson->asObject();

        CreateTransactionDTO dto;
        dto.tenantId = getTenantId(req);
        dto.workspaceId = bodyObj.count("workspace_id") ? bodyObj["workspace_id"]->asString() : "";
        dto.accountId = bodyObj["account_id"]->asString();
        dto.categoryId = bodyObj.count("category_id") ? bodyObj["category_id"]->asString() : "";
        dto.type = bodyObj["type"]->asString();
        dto.amount = bodyObj["amount"]->asNumber();
        dto.currency = bodyObj["currency"]->asString();
        dto.description = bodyObj.count("description") ? bodyObj["description"]->asString() : "";
        dto.transactionDate = bodyObj["transaction_date"]->asString();
        dto.referenceId = bodyObj.count("reference_id") ? bodyObj["reference_id"]->asString() : "";
        dto.referenceType = bodyObj.count("reference_type") ? bodyObj["reference_type"]->asString() : "";
        dto.createdBy = getUserId(req);

        Utils::Logger::info("[FinanceController::createTransaction] Calling Use Case for type=" + dto.type + ", amount=" + std::to_string(dto.amount));
        std::string txnId = createTransactionUseCase_->execute(dto);

        auto response = Core::Json::makeObject();
        response->asObject()["id"] = Core::Json::makeString(txnId);
        response->asObject()["message"] = Core::Json::makeString("Transaction created successfully");

        return Core::Http::Response(Core::Http::StatusCode::Created).json(*response);
    } catch (const std::exception& e) {
        Utils::Logger::info("[FinanceController::createTransaction] ❌ EXCEPTION CAUGHT: " + std::string(e.what()));
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::createTransfer(const Core::Http::Request& req) {
    try {
        auto bodyJson = req.getJson();        if (!bodyJson || !bodyJson->isObject()) {            auto json = Core::Json::makeObject();            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);        }        auto& bodyObj = bodyJson->asObject();

        CreateTransferDTO dto;
        dto.tenantId = getTenantId(req);
        dto.fromAccountId = bodyObj["from_account_id"]->asString();
        dto.toAccountId = bodyObj["to_account_id"]->asString();
        dto.amount = bodyObj["amount"]->asNumber();
        dto.description = bodyObj.count("description") ? bodyObj["description"]->asString() : "";
        dto.transactionDate = bodyObj["transaction_date"]->asString();
        dto.createdBy = getUserId(req);

        std::string transferId = createTransferUseCase_->execute(dto);

        auto response = Core::Json::makeObject();
        response->asObject()["id"] = Core::Json::makeString(transferId);
        response->asObject()["message"] = Core::Json::makeString("Transfer completed successfully");

        return Core::Http::Response(Core::Http::StatusCode::Created).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::getTransaction(const Core::Http::Request& req) {
    try {
        std::string txnId = req.getParam("id");
        std::string tenantId = getTenantId(req);

        auto txn = getTransactionUseCase_->execute(txnId, tenantId);

        auto response = Core::Json::makeObject();
        response->asObject()["id"] = Core::Json::makeString(txn.getId());
        response->asObject()["tenant_id"] = Core::Json::makeString(txn.getTenantId());
        response->asObject()["account_id"] = Core::Json::makeString(txn.getAccountId());
        response->asObject()["type"] = Core::Json::makeString(txn.getType().toString());
        response->asObject()["amount"] = Core::Json::makeNumber(txn.getAmount());
        response->asObject()["currency"] = Core::Json::makeString(txn.getCurrency());
        response->asObject()["status"] = Core::Json::makeString(txn.getStatus().toString());
        response->asObject()["transaction_date"] = Core::Json::makeString(txn.getTransactionDate());

        if (txn.getCategoryId().has_value()) {
            response->asObject()["category_id"] = Core::Json::makeString(*txn.getCategoryId());
        }
        if (txn.getDescription().has_value()) {
            response->asObject()["description"] = Core::Json::makeString(*txn.getDescription());
        }

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::listTransactions(const Core::Http::Request& req) {
    try {
        std::string tenantId = getTenantId(req);

        ListTransactionsFilters filters;
        filters.accountId = req.getQuery("account_id");
        filters.categoryId = req.getQuery("category_id");
        filters.status = req.getQuery("status");
        filters.startDate = req.getQuery("start_date");
        filters.endDate = req.getQuery("end_date");
        filters.type = req.getQuery("type");

        auto transactions = listTransactionsUseCase_->execute(tenantId, filters);

        auto txnsArray = Core::Json::makeArray();
        for (const auto& txn : transactions) {
            auto obj = Core::Json::makeObject();
            obj->asObject()["id"] = Core::Json::makeString(txn.getId());
            obj->asObject()["account_id"] = Core::Json::makeString(txn.getAccountId());
            obj->asObject()["type"] = Core::Json::makeString(txn.getType().toString());
            obj->asObject()["amount"] = Core::Json::makeNumber(txn.getAmount());
            obj->asObject()["currency"] = Core::Json::makeString(txn.getCurrency());
            obj->asObject()["status"] = Core::Json::makeString(txn.getStatus().toString());
            obj->asObject()["transaction_date"] = Core::Json::makeString(txn.getTransactionDate());
            txnsArray->asArray().push_back(obj);
        }

        auto response = Core::Json::makeObject();
        response->asObject()["transactions"] = txnsArray;
        response->asObject()["total"] = Core::Json::makeNumber(transactions.size());

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::updateTransaction(const Core::Http::Request& req) {
    try {
        std::string txnId = req.getParam("id");
        auto bodyJson = req.getJson();        if (!bodyJson || !bodyJson->isObject()) {            auto json = Core::Json::makeObject();            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);        }        auto& bodyObj = bodyJson->asObject();

        UpdateTransactionDTO dto;
        dto.transactionId = txnId;
        dto.tenantId = getTenantId(req);

        if (bodyObj.count("category_id")) dto.categoryId = bodyObj["category_id"]->asString();
        if (bodyObj.count("description")) dto.description = bodyObj["description"]->asString();

        updateTransactionUseCase_->execute(dto);

        auto response = Core::Json::makeObject();
        response->asObject()["message"] = Core::Json::makeString("Transaction updated successfully");

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::deleteTransaction(const Core::Http::Request& req) {
    try {
        std::string txnId = req.getParam("id");
        std::string tenantId = getTenantId(req);

        deleteTransactionUseCase_->execute(txnId, tenantId);

        auto response = Core::Json::makeObject();
        response->asObject()["message"] = Core::Json::makeString("Transaction deleted successfully");

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::completeTransaction(const Core::Http::Request& req) {
    try {
        std::string txnId = req.getParam("id");
        std::string tenantId = getTenantId(req);

        completeTransactionUseCase_->execute(txnId, tenantId);

        auto response = Core::Json::makeObject();
        response->asObject()["message"] = Core::Json::makeString("Transaction completed successfully");

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::cancelTransaction(const Core::Http::Request& req) {
    try {
        std::string txnId = req.getParam("id");
        std::string tenantId = getTenantId(req);

        cancelTransactionUseCase_->execute(txnId, tenantId);

        auto response = Core::Json::makeObject();
        response->asObject()["message"] = Core::Json::makeString("Transaction cancelled successfully");

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

// ========== CATEGORY ENDPOINTS (simplified - implement similarly) ==========
Core::Http::Response FinanceController::createCategory(const Core::Http::Request& req) {
    try {
        auto bodyJson = req.getJson();        if (!bodyJson || !bodyJson->isObject()) {            auto json = Core::Json::makeObject();            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);        }        auto& bodyObj = bodyJson->asObject();
        CreateCategoryDTO dto;
        dto.tenantId = getTenantId(req);
        dto.workspaceId = bodyObj.count("workspace_id") ? bodyObj["workspace_id"]->asString() : "";
        dto.name = bodyObj["name"]->asString();
        dto.type = bodyObj["type"]->asString();
        dto.parentId = bodyObj.count("parent_id") ? bodyObj["parent_id"]->asString() : "";
        dto.color = bodyObj.count("color") ? bodyObj["color"]->asString() : "";
        dto.icon = bodyObj.count("icon") ? bodyObj["icon"]->asString() : "";

        std::string catId = createCategoryUseCase_->execute(dto);

        auto response = Core::Json::makeObject();
        response->asObject()["id"] = Core::Json::makeString(catId);
        response->asObject()["message"] = Core::Json::makeString("Category created successfully");
        return Core::Http::Response(Core::Http::StatusCode::Created).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::getCategory(const Core::Http::Request& req) {
    try {
        std::string catId = req.getParam("id");
        auto cat = getCategoryUseCase_->execute(catId, getTenantId(req));

        auto response = Core::Json::makeObject();
        response->asObject()["id"] = Core::Json::makeString(cat.getId());
        response->asObject()["name"] = Core::Json::makeString(cat.getName());
        response->asObject()["type"] = Core::Json::makeString(cat.getType());
        response->asObject()["is_active"] = Core::Json::makeBool(cat.isActive());
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::listCategories(const Core::Http::Request& req) {
    try {
        std::string type = req.getQuery("type");
        bool rootOnly = req.getQuery("root_only") == "true";
        auto categories = listCategoriesUseCase_->execute(getTenantId(req), type, rootOnly);

        auto catsArray = Core::Json::makeArray();
        for (const auto& cat : categories) {
            auto obj = Core::Json::makeObject();
            obj->asObject()["id"] = Core::Json::makeString(cat.getId());
            obj->asObject()["name"] = Core::Json::makeString(cat.getName());
            obj->asObject()["type"] = Core::Json::makeString(cat.getType());
            catsArray->asArray().push_back(obj);
        }

        auto response = Core::Json::makeObject();
        response->asObject()["categories"] = catsArray;
        response->asObject()["total"] = Core::Json::makeNumber(categories.size());
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::updateCategory(const Core::Http::Request& req) {
    try {
        std::string catId = req.getParam("id");
        auto bodyJson = req.getJson();        if (!bodyJson || !bodyJson->isObject()) {            auto json = Core::Json::makeObject();            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);        }        auto& bodyObj = bodyJson->asObject();

        UpdateCategoryDTO dto;
        dto.categoryId = catId;
        dto.tenantId = getTenantId(req);
        if (bodyObj.count("name")) dto.name = bodyObj["name"]->asString();
        if (bodyObj.count("color")) dto.color = bodyObj["color"]->asString();
        if (bodyObj.count("icon")) dto.icon = bodyObj["icon"]->asString();

        updateCategoryUseCase_->execute(dto);

        auto response = Core::Json::makeObject();
        response->asObject()["message"] = Core::Json::makeString("Category updated successfully");
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::deleteCategory(const Core::Http::Request& req) {
    try {
        std::string catId = req.getParam("id");
        deleteCategoryUseCase_->execute(catId, getTenantId(req));

        auto response = Core::Json::makeObject();
        response->asObject()["message"] = Core::Json::makeString("Category deleted successfully");
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

// ========== BUDGET ENDPOINTS (simplified - implement similarly) ==========
Core::Http::Response FinanceController::createBudget(const Core::Http::Request& req) {
    try {
        auto bodyJson = req.getJson();        if (!bodyJson || !bodyJson->isObject()) {            auto json = Core::Json::makeObject();            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);        }        auto& bodyObj = bodyJson->asObject();
        CreateBudgetDTO dto;
        dto.tenantId = getTenantId(req);
        dto.workspaceId = bodyObj.count("workspace_id") ? bodyObj["workspace_id"]->asString() : "";
        dto.name = bodyObj["name"]->asString();
        dto.categoryId = bodyObj.count("category_id") ? bodyObj["category_id"]->asString() : "";
        dto.amount = bodyObj["amount"]->asNumber();
        dto.period = bodyObj["period"]->asString();
        dto.startDate = bodyObj["start_date"]->asString();
        dto.endDate = bodyObj["end_date"]->asString();
        dto.isRecurring = bodyObj.count("is_recurring") ? bodyObj["is_recurring"]->asBool() : false;
        dto.alertPercentage = bodyObj.count("alert_percentage") ? (int)bodyObj["alert_percentage"]->asNumber() : 80;
        dto.createdBy = getUserId(req);

        std::string budgetId = createBudgetUseCase_->execute(dto);

        auto response = Core::Json::makeObject();
        response->asObject()["id"] = Core::Json::makeString(budgetId);
        response->asObject()["message"] = Core::Json::makeString("Budget created successfully");
        return Core::Http::Response(Core::Http::StatusCode::Created).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::getBudget(const Core::Http::Request& req) {
    try {
        std::string budgetId = req.getParam("id");
        auto budget = getBudgetUseCase_->execute(budgetId, getTenantId(req));

        auto response = Core::Json::makeObject();
        response->asObject()["id"] = Core::Json::makeString(budget.getId());
        response->asObject()["name"] = Core::Json::makeString(budget.getName());
        response->asObject()["amount"] = Core::Json::makeNumber(budget.getAmount());
        response->asObject()["spent"] = Core::Json::makeNumber(budget.getSpent());
        response->asObject()["remaining"] = Core::Json::makeNumber(budget.getRemaining());
        response->asObject()["percentage_spent"] = Core::Json::makeNumber(budget.getPercentageSpent());
        response->asObject()["status"] = Core::Json::makeString(budget.getStatus());
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::listBudgets(const Core::Http::Request& req) {
    try {
        std::string categoryId = req.getQuery("category_id");
        std::string period = req.getQuery("period");
        bool activeOnly = req.getQuery("active") == "true";

        auto budgets = listBudgetsUseCase_->execute(getTenantId(req), categoryId, period, activeOnly);

        auto budgetsArray = Core::Json::makeArray();
        for (const auto& budget : budgets) {
            auto obj = Core::Json::makeObject();
            obj->asObject()["id"] = Core::Json::makeString(budget.getId());
            obj->asObject()["name"] = Core::Json::makeString(budget.getName());
            obj->asObject()["amount"] = Core::Json::makeNumber(budget.getAmount());
            obj->asObject()["spent"] = Core::Json::makeNumber(budget.getSpent());
            budgetsArray->asArray().push_back(obj);
        }

        auto response = Core::Json::makeObject();
        response->asObject()["budgets"] = budgetsArray;
        response->asObject()["total"] = Core::Json::makeNumber(budgets.size());
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::updateBudget(const Core::Http::Request& req) {
    try {
        std::string budgetId = req.getParam("id");
        auto bodyJson = req.getJson();        if (!bodyJson || !bodyJson->isObject()) {            auto json = Core::Json::makeObject();            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);        }        auto& bodyObj = bodyJson->asObject();

        UpdateBudgetDTO dto;
        dto.budgetId = budgetId;
        dto.tenantId = getTenantId(req);
        if (bodyObj.count("name")) dto.name = bodyObj["name"]->asString();
        if (bodyObj.count("amount")) dto.amount = bodyObj["amount"]->asNumber();

        updateBudgetUseCase_->execute(dto);

        auto response = Core::Json::makeObject();
        response->asObject()["message"] = Core::Json::makeString("Budget updated successfully");
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::deleteBudget(const Core::Http::Request& req) {
    try {
        std::string budgetId = req.getParam("id");
        deleteBudgetUseCase_->execute(budgetId, getTenantId(req));

        auto response = Core::Json::makeObject();
        response->asObject()["message"] = Core::Json::makeString("Budget deleted successfully");
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

// ========== REPORT ENDPOINTS ==========
Core::Http::Response FinanceController::getCashFlow(const Core::Http::Request& req) {
    try {
        std::string startDate = req.getQuery("start_date");
        std::string endDate = req.getQuery("end_date");
        std::string currency = req.getQuery("currency");
        if (currency.empty()) currency = "BRL";

        auto report = getCashFlowUseCase_->execute(getTenantId(req), startDate, endDate, currency);

        auto response = Core::Json::makeObject();
        response->asObject()["total_income"] = Core::Json::makeNumber(report.totalIncome);
        response->asObject()["total_expense"] = Core::Json::makeNumber(report.totalExpense);
        response->asObject()["net_cash_flow"] = Core::Json::makeNumber(report.netCashFlow);
        response->asObject()["start_date"] = Core::Json::makeString(report.startDate);
        response->asObject()["end_date"] = Core::Json::makeString(report.endDate);
        response->asObject()["currency"] = Core::Json::makeString(report.currency);

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::getIncomeVsExpense(const Core::Http::Request& req) {
    try {
        std::string startDate = req.getQuery("start_date");
        std::string endDate = req.getQuery("end_date");

        auto report = getIncomeVsExpenseUseCase_->execute(getTenantId(req), startDate, endDate);

        auto response = Core::Json::makeObject();
        response->asObject()["total_income"] = Core::Json::makeNumber(report.totalIncome);
        response->asObject()["total_expense"] = Core::Json::makeNumber(report.totalExpense);
        response->asObject()["difference"] = Core::Json::makeNumber(report.difference);
        response->asObject()["percentage_income"] = Core::Json::makeNumber(report.percentageIncome);
        response->asObject()["percentage_expense"] = Core::Json::makeNumber(report.percentageExpense);

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::getCategoryReport(const Core::Http::Request& req) {
    try {
        std::string type = req.getQuery("type");
        std::string startDate = req.getQuery("start_date");
        std::string endDate = req.getQuery("end_date");

        auto items = getCategoryReportUseCase_->execute(getTenantId(req), type, startDate, endDate);

        auto itemsArray = Core::Json::makeArray();
        for (const auto& item : items) {
            auto obj = Core::Json::makeObject();
            obj->asObject()["category"] = Core::Json::makeString(item.categoryName);
            obj->asObject()["total"] = Core::Json::makeNumber(item.total);
            obj->asObject()["percentage"] = Core::Json::makeNumber(item.percentage);
            itemsArray->asArray().push_back(obj);
        }

        auto response = Core::Json::makeObject();
        response->asObject()["items"] = itemsArray;
        response->asObject()["type"] = Core::Json::makeString(type);

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::getProfitLoss(const Core::Http::Request& req) {
    try {
        std::string startDate = req.getQuery("start_date");
        std::string endDate = req.getQuery("end_date");

        auto report = getProfitLossUseCase_->execute(getTenantId(req), startDate, endDate);

        auto response = Core::Json::makeObject();
        response->asObject()["total_revenue"] = Core::Json::makeNumber(report.totalRevenue);
        response->asObject()["total_expenses"] = Core::Json::makeNumber(report.totalExpenses);
        response->asObject()["gross_profit"] = Core::Json::makeNumber(report.grossProfit);
        response->asObject()["net_profit"] = Core::Json::makeNumber(report.netProfit);
        response->asObject()["profit_margin"] = Core::Json::makeNumber(report.profitMargin);

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

Core::Http::Response FinanceController::getBudgetPerformance(const Core::Http::Request& req) {
    try {
        std::string date = req.getQuery("date");
        
        auto items = getBudgetPerformanceUseCase_->execute(getTenantId(req), date);

        auto itemsArray = Core::Json::makeArray();
        for (const auto& item : items) {
            auto obj = Core::Json::makeObject();
            obj->asObject()["budget_id"] = Core::Json::makeString(item.budgetId);
            obj->asObject()["budget_name"] = Core::Json::makeString(item.budgetName);
            obj->asObject()["budget_amount"] = Core::Json::makeNumber(item.budgetAmount);
            obj->asObject()["spent"] = Core::Json::makeNumber(item.spent);
            obj->asObject()["remaining"] = Core::Json::makeNumber(item.remaining);
            obj->asObject()["percentage_used"] = Core::Json::makeNumber(item.percentageUsed);
            obj->asObject()["status"] = Core::Json::makeString(item.status);
            obj->asObject()["should_alert"] = Core::Json::makeBool(item.shouldAlert);
            itemsArray->asArray().push_back(obj);
        }

        auto response = Core::Json::makeObject();
        response->asObject()["budgets"] = itemsArray;
        response->asObject()["total"] = Core::Json::makeNumber(items.size());

        return Core::Http::Response(Core::Http::StatusCode::OK).json(*response);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["error"] = Core::Json::makeString(e.what());
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*error);
    }
}

} // namespace Finance

