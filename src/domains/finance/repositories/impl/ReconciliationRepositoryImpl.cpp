#include "ReconciliationRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {

namespace Utils = Core::Utils;

ReconciliationRepositoryImpl::ReconciliationRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {}

bool ReconciliationRepositoryImpl::save(const Reconciliation& reconciliation) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        INSERT INTO reconciliations 
        (id, tenant_id, workspace_id, account_id, statement_date, statement_balance, 
         system_balance, difference, status, matched_transactions, unmatched_transactions, 
         notes, reconciled_by, reconciled_at, created_by, created_at, updated_at)
        VALUES ($1, $2, NULLIF($3,''), $4, $5, $6, $7, $8, $9, $10, $11, NULLIF($12,''), 
                NULLIF($13,''), NULLIF($14,''), $15, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
    )";
    
    std::vector<std::string> params = {
        reconciliation.getId(),
        reconciliation.getTenantId(),
        reconciliation.getWorkspaceId(),
        reconciliation.getAccountId(),
        reconciliation.getStatementDate(),
        std::to_string(reconciliation.getStatementBalance()),
        std::to_string(reconciliation.getSystemBalance()),
        std::to_string(reconciliation.getDifference()),
        reconciliation.getStatus().toString(),
        std::to_string(reconciliation.getMatchedTransactions()),
        std::to_string(reconciliation.getUnmatchedTransactions()),
        reconciliation.getNotes().value_or(""),
        reconciliation.getReconciledBy().value_or(""),
        reconciliation.getReconciledAt().value_or(""),
        reconciliation.getCreatedBy()
    };
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        Utils::Logger::error("[ReconciliationRepository::save] Failed to save reconciliation: " + reconciliation.getId());
        return false;
    }
    
    Utils::Logger::info("[ReconciliationRepository::save] Reconciliation saved successfully: " + reconciliation.getId() + 
                        " (Account: " + reconciliation.getAccountId() + ", Date: " + reconciliation.getStatementDate() + ")");
    return true;
}

bool ReconciliationRepositoryImpl::update(const Reconciliation& reconciliation) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE reconciliations 
        SET workspace_id = NULLIF($3,''),
            statement_balance = $4,
            system_balance = $5,
            difference = $6,
            status = $7,
            matched_transactions = $8,
            unmatched_transactions = $9,
            notes = NULLIF($10,''),
            reconciled_by = NULLIF($11,''),
            reconciled_at = NULLIF($12,''),
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $1 AND tenant_id = $2
    )";
    
    std::vector<std::string> params = {
        reconciliation.getId(),
        reconciliation.getTenantId(),
        reconciliation.getWorkspaceId(),
        std::to_string(reconciliation.getStatementBalance()),
        std::to_string(reconciliation.getSystemBalance()),
        std::to_string(reconciliation.getDifference()),
        reconciliation.getStatus().toString(),
        std::to_string(reconciliation.getMatchedTransactions()),
        std::to_string(reconciliation.getUnmatchedTransactions()),
        reconciliation.getNotes().value_or(""),
        reconciliation.getReconciledBy().value_or(""),
        reconciliation.getReconciledAt().value_or("")
    };
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        Utils::Logger::error("[ReconciliationRepository::update] Failed to update reconciliation: " + reconciliation.getId());
        return false;
    }
    
    Utils::Logger::info("[ReconciliationRepository::update] Reconciliation updated successfully: " + reconciliation.getId());
    return true;
}

bool ReconciliationRepositoryImpl::remove(const std::string& id) {
    auto conn = pool_->acquire();
    
    std::string query = "DELETE FROM reconciliations WHERE id = $1";
    std::vector<std::string> params = {id};
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        Utils::Logger::error("[ReconciliationRepository::remove] Failed to remove reconciliation: " + id);
        return false;
    }
    
    Utils::Logger::info("[ReconciliationRepository::remove] Reconciliation removed successfully: " + id);
    return true;
}

std::optional<Reconciliation> ReconciliationRepositoryImpl::findById(const std::string& id, const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM reconciliations WHERE id = $1 AND tenant_id = $2";
    std::vector<std::string> params = {id, tenantId};
    
    auto result = conn->executeParams(query, params);
    
    if (result.rowCount() == 0) {
        Utils::Logger::debug("[ReconciliationRepository::findById] Reconciliation not found: " + id);
        return std::nullopt;
    }
    
    return mapToReconciliation(result, 0);
}

std::vector<Reconciliation> ReconciliationRepositoryImpl::findByAccount(const std::string& accountId, const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT * FROM reconciliations 
        WHERE account_id = $1 AND tenant_id = $2 
        ORDER BY statement_date DESC
    )";
    
    std::vector<std::string> params = {accountId, tenantId};
    
    auto result = conn->executeParams(query, params);
    
    std::vector<Reconciliation> reconciliations;
    for (int i = 0; i < result.rowCount(); i++) {
        reconciliations.push_back(mapToReconciliation(result, i));
    }
    
    Utils::Logger::debug("[ReconciliationRepository::findByAccount] Found " + std::to_string(reconciliations.size()) + 
                         " reconciliations for account: " + accountId);
    return reconciliations;
}

std::vector<Reconciliation> ReconciliationRepositoryImpl::findByStatus(const std::string& status, const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT * FROM reconciliations 
        WHERE status = $1 AND tenant_id = $2 
        ORDER BY statement_date DESC
    )";
    
    std::vector<std::string> params = {status, tenantId};
    
    auto result = conn->executeParams(query, params);
    
    std::vector<Reconciliation> reconciliations;
    for (int i = 0; i < result.rowCount(); i++) {
        reconciliations.push_back(mapToReconciliation(result, i));
    }
    
    Utils::Logger::debug("[ReconciliationRepository::findByStatus] Found " + std::to_string(reconciliations.size()) + 
                         " reconciliations with status: " + status);
    return reconciliations;
}

std::optional<Reconciliation> ReconciliationRepositoryImpl::findLatestByAccount(const std::string& accountId, const std::string& tenantId) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT * FROM reconciliations 
        WHERE account_id = $1 AND tenant_id = $2 
        ORDER BY statement_date DESC 
        LIMIT 1
    )";
    
    std::vector<std::string> params = {accountId, tenantId};
    
    auto result = conn->executeParams(query, params);
    
    if (result.rowCount() == 0) {
        Utils::Logger::debug("[ReconciliationRepository::findLatestByAccount] No reconciliation found for account: " + accountId);
        return std::nullopt;
    }
    
    auto reconciliation = mapToReconciliation(result, 0);
    Utils::Logger::debug("[ReconciliationRepository::findLatestByAccount] Latest reconciliation found: " + reconciliation.getId() + 
                         " (Date: " + reconciliation.getStatementDate() + ")");
    return reconciliation;
}

Reconciliation ReconciliationRepositoryImpl::mapToReconciliation(const Core::Database::QueryResult& result, int row) {
    // id, tenant_id, workspace_id, account_id, statement_date, statement_balance, system_balance, difference,
    // status, matched_transactions, unmatched_transactions, notes, reconciled_by, reconciled_at, created_by, created_at, updated_at
    
    std::string id = result.getValue(row, 0);
    std::string tenantId = result.getValue(row, 1);
    std::string workspaceId = result.getValue(row, 2);
    std::string accountId = result.getValue(row, 3);
    std::string statementDate = result.getValue(row, 4);
    double statementBalance = std::stod(result.getValue(row, 5));
    double systemBalance = std::stod(result.getValue(row, 6));
    double difference = std::stod(result.getValue(row, 7));
    std::string statusStr = result.getValue(row, 8);
    int matchedTransactions = std::stoi(result.getValue(row, 9));
    int unmatchedTransactions = std::stoi(result.getValue(row, 10));
    std::string notes = result.getValue(row, 11);
    std::string reconciledBy = result.getValue(row, 12);
    std::string reconciledAt = result.getValue(row, 13);
    std::string createdBy = result.getValue(row, 14);
    
    Reconciliation reconciliation(
        id,
        tenantId,
        accountId,
        statementDate,
        statementBalance,
        systemBalance,
        difference,
        ReconciliationStatus(statusStr),
        matchedTransactions,
        unmatchedTransactions,
        createdBy
    );
    
    reconciliation.setWorkspaceId(workspaceId);
    
    if (!notes.empty()) {
        reconciliation.setNotes(notes);
    }
    
    return reconciliation;
}

} // namespace Finance

