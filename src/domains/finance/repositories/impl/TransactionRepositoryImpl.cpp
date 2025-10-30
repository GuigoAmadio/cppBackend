#include "TransactionRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {

namespace Utils = Core::Utils;

TransactionRepositoryImpl::TransactionRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {}

bool TransactionRepositoryImpl::save(const Transaction& txn) {
    auto conn = pool_->acquire();
    
    Utils::Logger::info("[TransactionRepository::save] Starting save for transaction: " + txn.getId());
    Utils::Logger::info("[TransactionRepository::save] Type: " + txn.getType().toString() + ", Amount: " + std::to_string(txn.getAmount()));
    Utils::Logger::info("[TransactionRepository::save] Category ID: " + (txn.getCategoryId().has_value() ? *txn.getCategoryId() : "NULL"));
    
    std::string query = R"(
        INSERT INTO financial_transactions 
        (id, tenant_id, workspace_id, account_id, category_id, type, amount, currency, status, 
         description, transaction_date, reference_id, reference_type, from_account_id, to_account_id, created_by)
        VALUES ($1, $2, NULLIF($3,''), $4, NULLIF($5,''), $6, $7, $8, $9, NULLIF($10,''), $11, 
                NULLIF($12,''), NULLIF($13,''), NULLIF($14,''), NULLIF($15,''), $16)
    )";
    
    std::vector<std::string> params = {
        txn.getId(), txn.getTenantId(), txn.getWorkspaceId(),
        txn.getAccountId(), txn.getCategoryId().value_or(""),
        txn.getType().toString(), std::to_string(txn.getAmount()),
        txn.getCurrency(), txn.getStatus().toString(),
        txn.getDescription().value_or(""), txn.getTransactionDate(),
        txn.getReferenceId().value_or(""), txn.getReferenceType().value_or(""),
        txn.getFromAccountId().value_or(""), txn.getToAccountId().value_or(""),
        txn.getCreatedBy()
    };
    
    Utils::Logger::info("[TransactionRepository::save] Executing INSERT with " + std::to_string(params.size()) + " parameters");
    auto result = conn->executeParams(query, params);
    bool success = result.isSuccess();
    
    if (!success) {
        Utils::Logger::info("[TransactionRepository::save] ❌ INSERT FAILED! Check PostgreSQL logs for details.");
    } else {
        Utils::Logger::info("[TransactionRepository::save] ✅ INSERT SUCCESS!");
    }
    
    return success;
}

bool TransactionRepositoryImpl::update(const Transaction& txn) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE financial_transactions 
        SET category_id = NULLIF($3,''), status = $4, description = NULLIF($5,''),
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $1 AND tenant_id = $2
    )";
    
    std::vector<std::string> params = {
        txn.getId(), txn.getTenantId(),
        txn.getCategoryId().value_or(""),
        txn.getStatus().toString(),
        txn.getDescription().value_or("")
    };
    
    return conn->executeParams(query, params).isSuccess();
}

bool TransactionRepositoryImpl::remove(const std::string& id) {
    auto conn = pool_->acquire();
    return conn->executeParams("DELETE FROM financial_transactions WHERE id = $1", {id}).isSuccess();
}

std::optional<Transaction> TransactionRepositoryImpl::findById(const std::string& id, const std::string& tenantId) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams(
        "SELECT * FROM financial_transactions WHERE id = $1 AND tenant_id = $2",
        {id, tenantId}
    );
    
    if (result.rowCount() == 0) return std::nullopt;
    return mapToTransaction(result, 0);
}

std::vector<Transaction> TransactionRepositoryImpl::findByTenant(const std::string& tenantId) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams(
        "SELECT * FROM financial_transactions WHERE tenant_id = $1 ORDER BY transaction_date DESC",
        {tenantId}
    );
    
    std::vector<Transaction> transactions;
    for (int i = 0; i < result.rowCount(); i++) {
        transactions.push_back(mapToTransaction(result, i));
    }
    return transactions;
}

std::vector<Transaction> TransactionRepositoryImpl::findByAccount(const std::string& accountId, const std::string& tenantId) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams(
        "SELECT * FROM financial_transactions WHERE account_id = $1 AND tenant_id = $2 ORDER BY transaction_date DESC",
        {accountId, tenantId}
    );
    
    std::vector<Transaction> transactions;
    for (int i = 0; i < result.rowCount(); i++) {
        transactions.push_back(mapToTransaction(result, i));
    }
    return transactions;
}

std::vector<Transaction> TransactionRepositoryImpl::findByCategory(const std::string& categoryId, const std::string& tenantId) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams(
        "SELECT * FROM financial_transactions WHERE category_id = $1 AND tenant_id = $2 ORDER BY transaction_date DESC",
        {categoryId, tenantId}
    );
    
    std::vector<Transaction> transactions;
    for (int i = 0; i < result.rowCount(); i++) {
        transactions.push_back(mapToTransaction(result, i));
    }
    return transactions;
}

std::vector<Transaction> TransactionRepositoryImpl::findByDateRange(
    const std::string& startDate,
    const std::string& endDate,
    const std::string& tenantId
) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams(
        "SELECT * FROM financial_transactions WHERE transaction_date BETWEEN $1 AND $2 AND tenant_id = $3 ORDER BY transaction_date DESC",
        {startDate, endDate, tenantId}
    );
    
    std::vector<Transaction> transactions;
    for (int i = 0; i < result.rowCount(); i++) {
        transactions.push_back(mapToTransaction(result, i));
    }
    return transactions;
}

std::vector<Transaction> TransactionRepositoryImpl::findByStatus(const std::string& status, const std::string& tenantId) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams(
        "SELECT * FROM financial_transactions WHERE status = $1 AND tenant_id = $2 ORDER BY transaction_date DESC",
        {status, tenantId}
    );
    
    std::vector<Transaction> transactions;
    for (int i = 0; i < result.rowCount(); i++) {
        transactions.push_back(mapToTransaction(result, i));
    }
    return transactions;
}

std::vector<Transaction> TransactionRepositoryImpl::findByReference(
    const std::string& referenceId,
    const std::string& referenceType,
    const std::string& tenantId
) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams(
        "SELECT * FROM financial_transactions WHERE reference_id = $1 AND reference_type = $2 AND tenant_id = $3",
        {referenceId, referenceType, tenantId}
    );
    
    std::vector<Transaction> transactions;
    for (int i = 0; i < result.rowCount(); i++) {
        transactions.push_back(mapToTransaction(result, i));
    }
    return transactions;
}

double TransactionRepositoryImpl::getTotalByType(
    const std::string& type,
    const std::string& tenantId,
    const std::string& startDate,
    const std::string& endDate
) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams(
        "SELECT COALESCE(SUM(amount), 0) FROM financial_transactions WHERE type = $1 AND tenant_id = $2 AND transaction_date BETWEEN $3 AND $4 AND status = 'completed'",
        {type, tenantId, startDate, endDate}
    );
    
    if (result.rowCount() == 0) return 0.0;
    return std::stod(result.getValue(0, 0));
}

double TransactionRepositoryImpl::getTotalByCategory(
    const std::string& categoryId,
    const std::string& tenantId,
    const std::string& startDate,
    const std::string& endDate
) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams(
        "SELECT COALESCE(SUM(amount), 0) FROM financial_transactions WHERE category_id = $1 AND tenant_id = $2 AND transaction_date BETWEEN $3 AND $4 AND status = 'completed'",
        {categoryId, tenantId, startDate, endDate}
    );
    
    if (result.rowCount() == 0) return 0.0;
    return std::stod(result.getValue(0, 0));
}

std::map<std::string, double> TransactionRepositoryImpl::getTotalsByCategory(
    const std::string& type,
    const std::string& tenantId,
    const std::string& startDate,
    const std::string& endDate
) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams(
        R"(
            SELECT tc.name, COALESCE(SUM(ft.amount), 0) as total
            FROM financial_transactions ft
            JOIN transaction_categories tc ON ft.category_id = tc.id
            WHERE ft.type = $1 AND ft.tenant_id = $2 
              AND ft.transaction_date BETWEEN $3 AND $4 
              AND ft.status = 'completed'
            GROUP BY tc.name
            ORDER BY total DESC
        )",
        {type, tenantId, startDate, endDate}
    );
    
    std::map<std::string, double> totals;
    for (int i = 0; i < result.rowCount(); i++) {
        totals[result.getValue(i, 0)] = std::stod(result.getValue(i, 1));
    }
    return totals;
}

std::map<std::string, double> TransactionRepositoryImpl::getCashFlow(
    const std::string& tenantId,
    const std::string& startDate,
    const std::string& endDate
) {
    auto conn = pool_->acquire();
    
    // Income
    auto incomeResult = conn->executeParams(
        "SELECT COALESCE(SUM(amount), 0) FROM financial_transactions WHERE type = 'income' AND tenant_id = $1 AND transaction_date BETWEEN $2 AND $3 AND status = 'completed'",
        {tenantId, startDate, endDate}
    );
    
    // Expense
    auto expenseResult = conn->executeParams(
        "SELECT COALESCE(SUM(amount), 0) FROM financial_transactions WHERE type = 'expense' AND tenant_id = $1 AND transaction_date BETWEEN $2 AND $3 AND status = 'completed'",
        {tenantId, startDate, endDate}
    );
    
    double income = incomeResult.rowCount() > 0 ? std::stod(incomeResult.getValue(0, 0)) : 0.0;
    double expense = expenseResult.rowCount() > 0 ? std::stod(expenseResult.getValue(0, 0)) : 0.0;
    
    return {
        {"income", income},
        {"expense", expense},
        {"net", income - expense}
    };
}

Transaction TransactionRepositoryImpl::mapToTransaction(const Core::Database::QueryResult& result, int row) {
    // id, tenant_id, workspace_id, account_id, category_id, type, amount, currency, status, description, transaction_date, reference_id, reference_type, from_account_id, to_account_id, reconciliation_id, reconciled_at, created_by, created_at, updated_at
    
    Transaction txn(
        result.getValue(row, 0),  // id
        result.getValue(row, 1),  // tenant_id
        result.getValue(row, 3),  // account_id
        TransactionType(result.getValue(row, 5)),  // type
        std::stod(result.getValue(row, 6)),  // amount
        result.getValue(row, 7),  // currency
        TransactionStatus(result.getValue(row, 8)),  // status
        result.getValue(row, 10),  // transaction_date
        result.getValue(row, 17)  // created_by
    );
    
    txn.setWorkspaceId(result.getValue(row, 2));
    
    std::string categoryId = result.getValue(row, 4);
    if (!categoryId.empty()) txn.setCategoryId(categoryId);
    
    std::string description = result.getValue(row, 9);
    if (!description.empty()) txn.setDescription(description);
    
    std::string refId = result.getValue(row, 11);
    if (!refId.empty()) txn.setReferenceId(refId);
    
    std::string refType = result.getValue(row, 12);
    if (!refType.empty()) txn.setReferenceType(refType);
    
    std::string fromAccountId = result.getValue(row, 13);
    if (!fromAccountId.empty()) txn.setFromAccountId(fromAccountId);
    
    std::string toAccountId = result.getValue(row, 14);
    if (!toAccountId.empty()) txn.setToAccountId(toAccountId);
    
    return txn;
}

} // namespace Finance

