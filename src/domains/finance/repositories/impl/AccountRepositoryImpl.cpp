#include "AccountRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"
#include <sstream>

namespace Finance {

namespace Utils = Core::Utils;

AccountRepositoryImpl::AccountRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {}

bool AccountRepositoryImpl::save(const Account& account) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        INSERT INTO financial_accounts 
        (id, tenant_id, workspace_id, name, type, currency, balance, initial_balance, 
         bank_name, account_number, is_active, created_by, created_at, updated_at)
        VALUES ($1, $2, NULLIF($3,''), $4, $5, $6, $7, $8, NULLIF($9,''), NULLIF($10,''), $11, $12, 
                CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
    )";
    
    std::vector<std::string> params = {
        account.getId(),
        account.getTenantId(),
        account.getWorkspaceId(),
        account.getName(),
        account.getType().toString(),
        account.getCurrency(),
        std::to_string(account.getBalance()),
        std::to_string(account.getInitialBalance()),
        account.getBankName().value_or(""),
        account.getAccountNumber().value_or(""),
        account.isActive() ? "true" : "false",
        account.getCreatedBy()
    };
    
    auto result = conn->executeParams(query, params);
    return result.isSuccess();
}

bool AccountRepositoryImpl::update(const Account& account) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE financial_accounts 
        SET workspace_id = NULLIF($3,''),
            name = $4,
            type = $5,
            currency = $6,
            balance = $7,
            bank_name = NULLIF($8,''),
            account_number = NULLIF($9,''),
            is_active = $10,
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $1 AND tenant_id = $2
    )";
    
    std::vector<std::string> params = {
        account.getId(),
        account.getTenantId(),
        account.getWorkspaceId(),
        account.getName(),
        account.getType().toString(),
        account.getCurrency(),
        std::to_string(account.getBalance()),
        account.getBankName().value_or(""),
        account.getAccountNumber().value_or(""),
        account.isActive() ? "true" : "false"
    };
    
    auto result = conn->executeParams(query, params);
    return result.isSuccess();
}

bool AccountRepositoryImpl::remove(const std::string& id) {
    auto conn = pool_->acquire();
    std::string query = "DELETE FROM financial_accounts WHERE id = $1";
    std::vector<std::string> params = {id};
    auto result = conn->executeParams(query, params);
    return result.isSuccess();
}

std::optional<Account> AccountRepositoryImpl::findById(const std::string& id, const std::string& tenantId) {
    auto conn = pool_->acquire();
    std::string query = "SELECT * FROM financial_accounts WHERE id = $1 AND tenant_id = $2";
    std::vector<std::string> params = {id, tenantId};
    auto result = conn->executeParams(query, params);
    
    if (result.rowCount() == 0) return std::nullopt;
    return mapToAccount(result, 0);
}

std::vector<Account> AccountRepositoryImpl::findByTenant(const std::string& tenantId) {
    auto conn = pool_->acquire();
    std::string query = "SELECT * FROM financial_accounts WHERE tenant_id = $1 ORDER BY name";
    std::vector<std::string> params = {tenantId};
    auto result = conn->executeParams(query, params);
    
    std::vector<Account> accounts;
    for (int i = 0; i < result.rowCount(); i++) {
        accounts.push_back(mapToAccount(result, i));
    }
    return accounts;
}

std::vector<Account> AccountRepositoryImpl::findByWorkspace(const std::string& workspaceId, const std::string& tenantId) {
    auto conn = pool_->acquire();
    std::string query = "SELECT * FROM financial_accounts WHERE workspace_id = $1 AND tenant_id = $2 ORDER BY name";
    std::vector<std::string> params = {workspaceId, tenantId};
    auto result = conn->executeParams(query, params);
    
    std::vector<Account> accounts;
    for (int i = 0; i < result.rowCount(); i++) {
        accounts.push_back(mapToAccount(result, i));
    }
    return accounts;
}

std::vector<Account> AccountRepositoryImpl::findByCurrency(const std::string& currency, const std::string& tenantId) {
    auto conn = pool_->acquire();
    std::string query = "SELECT * FROM financial_accounts WHERE currency = $1 AND tenant_id = $2 ORDER BY name";
    std::vector<std::string> params = {currency, tenantId};
    auto result = conn->executeParams(query, params);
    
    std::vector<Account> accounts;
    for (int i = 0; i < result.rowCount(); i++) {
        accounts.push_back(mapToAccount(result, i));
    }
    return accounts;
}

std::vector<Account> AccountRepositoryImpl::findActive(const std::string& tenantId) {
    auto conn = pool_->acquire();
    std::string query = "SELECT * FROM financial_accounts WHERE is_active = true AND tenant_id = $1 ORDER BY name";
    std::vector<std::string> params = {tenantId};
    auto result = conn->executeParams(query, params);
    
    std::vector<Account> accounts;
    for (int i = 0; i < result.rowCount(); i++) {
        accounts.push_back(mapToAccount(result, i));
    }
    return accounts;
}

bool AccountRepositoryImpl::updateBalance(const std::string& id, double newBalance) {
    auto conn = pool_->acquire();
    std::string query = "UPDATE financial_accounts SET balance = $2, updated_at = CURRENT_TIMESTAMP WHERE id = $1";
    std::vector<std::string> params = {id, std::to_string(newBalance)};
    auto result = conn->executeParams(query, params);
    return result.isSuccess();
}

double AccountRepositoryImpl::getTotalBalance(const std::string& tenantId, const std::string& currency) {
    auto conn = pool_->acquire();
    std::string query = "SELECT COALESCE(SUM(balance), 0) as total FROM financial_accounts WHERE tenant_id = $1 AND currency = $2 AND is_active = true";
    std::vector<std::string> params = {tenantId, currency};
    auto result = conn->executeParams(query, params);
    
    if (result.rowCount() == 0) return 0.0;
    return std::stod(result.getValue(0, 0));
}

Account AccountRepositoryImpl::mapToAccount(const Core::Database::QueryResult& result, int row) {
    std::string id = result.getValue(row, 0);
    std::string tenantId = result.getValue(row, 1);
    std::string workspaceId = result.getValue(row, 2);
    std::string name = result.getValue(row, 3);
    std::string typeStr = result.getValue(row, 4);
    std::string currency = result.getValue(row, 5);
    double balance = std::stod(result.getValue(row, 6));
    double initialBalance = std::stod(result.getValue(row, 7));
    std::string bankName = result.getValue(row, 8);
    std::string accountNumber = result.getValue(row, 9);
    bool isActive = result.getValue(row, 10) == "t" || result.getValue(row, 10) == "true" || result.getValue(row, 10) == "1";
    std::string createdBy = result.getValue(row, 11);
    
    Account account(id, tenantId, name, AccountType(typeStr), currency, balance, initialBalance, isActive, createdBy);
    account.setWorkspaceId(workspaceId);
    if (!bankName.empty()) account.setBankName(bankName);
    if (!accountNumber.empty()) account.setAccountNumber(accountNumber);
    
    return account;
}

} // namespace Finance

