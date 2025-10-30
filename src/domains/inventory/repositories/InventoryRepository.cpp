#include "InventoryRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>

namespace Domain {
namespace Inventory {

InventoryTransaction InventoryRepositoryImpl::mapToTransaction(
    const Core::Database::QueryResult& result, 
    int row
) {
    LOG_DEBUG("[InventoryRepository] Mapping row " + std::to_string(row) + " to InventoryTransaction");
    
    std::string id = result.getValue(row, 0);
    std::string tenantId = result.getValue(row, 1);
    std::string productId = result.getValue(row, 2);
    std::string typeStr = result.getValue(row, 4);
    int quantity = std::stoi(result.getValue(row, 5));
    int quantityBefore = std::stoi(result.getValue(row, 6));
    int quantityAfter = std::stoi(result.getValue(row, 7));
    std::string createdBy = result.getValue(row, 10);
    
    InventoryTransaction transaction(
        id, tenantId, productId,
        TransactionType(typeStr),
        quantity, quantityBefore, quantityAfter,
        createdBy
    );
    
    // Campos opcionais
    std::string locationId = result.getValue(row, 3);
    if (!locationId.empty()) {
        transaction.setLocationId(locationId);
    }
    
    std::string refType = result.getValue(row, 8);
    if (!refType.empty()) {
        transaction.setReferenceType(ReferenceType(refType));
    }
    
    std::string refId = result.getValue(row, 9);
    if (!refId.empty()) {
        transaction.setReferenceId(refId);
    }
    
    std::string createdAt = result.getValue(row, 11);
    if (!createdAt.empty()) {
        transaction.setCreatedAt(createdAt);
    }
    
    return transaction;
}

bool InventoryRepositoryImpl::save(const InventoryTransaction& transaction) {
    LOG_DEBUG("[InventoryRepository] save() called - transaction_id: " + transaction.getId());
    LOG_DEBUG("[InventoryRepository] product_id: " + transaction.getProductId());
    LOG_DEBUG("[InventoryRepository] type: " + transaction.getType().toString());
    LOG_DEBUG("[InventoryRepository] quantity: " + std::to_string(transaction.getQuantity()));
    
    auto conn = pool_->acquire();
    LOG_DEBUG("[InventoryRepository] Connection acquired");
    
    std::string query = R"(
        INSERT INTO inventory_transactions (
            id, tenant_id, product_id, location_id, type,
            quantity, quantity_before, quantity_after,
            reference_type, reference_id, notes, created_by, created_at
        ) VALUES (
            $1, $2, $3, NULLIF($4,''), $5, $6, $7, $8, NULLIF($9,''), NULLIF($10,''), NULLIF($11,''), $12, CURRENT_TIMESTAMP
        )
    )";
    
    std::vector<std::string> params;
    params.push_back(transaction.getId());
    params.push_back(transaction.getTenantId());
    params.push_back(transaction.getProductId());
    params.push_back(transaction.getLocationId().value_or(""));  // NULL se vazio
    params.push_back(transaction.getType().toString());
    params.push_back(std::to_string(transaction.getQuantity()));
    params.push_back(std::to_string(transaction.getQuantityBefore()));
    params.push_back(std::to_string(transaction.getQuantityAfter()));
    params.push_back(transaction.getReferenceType().has_value() 
        ? transaction.getReferenceType()->toString() : "");
    params.push_back(transaction.getReferenceId().value_or(""));
    params.push_back(transaction.getNotes().value_or(""));
    params.push_back(transaction.getCreatedBy());
    
    LOG_DEBUG("[InventoryRepository] Parameters prepared, total: " + std::to_string(params.size()));
    LOG_DEBUG("[InventoryRepository] Executing INSERT query...");
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        LOG_ERROR("[InventoryRepository] INSERT failed!");
        LOG_ERROR("[InventoryRepository] Error: " + result.getError());
        return false;
    }
    
    LOG_DEBUG("[InventoryRepository] Transaction saved successfully");
    return true;
}

std::optional<InventoryTransaction> InventoryRepositoryImpl::findById(const std::string& id) {
    LOG_DEBUG("[InventoryRepository] findById() called - id: " + id);
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, product_id, location_id, type,
               quantity, quantity_before, quantity_after,
               reference_type, reference_id, created_by, created_at
        FROM inventory_transactions
        WHERE id = $1
    )";
    
    auto result = conn->executeParams(query, {id});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        LOG_DEBUG("[InventoryRepository] Transaction not found");
        return std::nullopt;
    }
    
    LOG_DEBUG("[InventoryRepository] Transaction found");
    return mapToTransaction(result, 0);
}

std::vector<InventoryTransaction> InventoryRepositoryImpl::findByProduct(
    const std::string& tenantId,
    const std::string& productId,
    int limit
) {
    LOG_DEBUG("[InventoryRepository] findByProduct() - tenant: " + tenantId + ", product: " + productId);
    
    std::vector<InventoryTransaction> transactions;
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, product_id, location_id, type,
               quantity, quantity_before, quantity_after,
               reference_type, reference_id, created_by, created_at
        FROM inventory_transactions
        WHERE tenant_id = $1 AND product_id = $2
        ORDER BY created_at DESC
        LIMIT $3
    )";
    
    auto result = conn->executeParams(query, {tenantId, productId, std::to_string(limit)});
    
    if (!result.isSuccess()) {
        LOG_ERROR("[InventoryRepository] Query failed: " + result.getError());
        return transactions;
    }
    
    LOG_DEBUG("[InventoryRepository] Found " + std::to_string(result.rowCount()) + " transactions");
    
    for (int i = 0; i < result.rowCount(); i++) {
        transactions.push_back(mapToTransaction(result, i));
    }
    
    return transactions;
}

std::vector<InventoryTransaction> InventoryRepositoryImpl::findByProductAndLocation(
    const std::string& tenantId,
    const std::string& productId,
    const std::string& locationId,
    int limit
) {
    LOG_DEBUG("[InventoryRepository] findByProductAndLocation()");
    
    std::vector<InventoryTransaction> transactions;
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, product_id, location_id, type,
               quantity, quantity_before, quantity_after,
               reference_type, reference_id, created_by, created_at
        FROM inventory_transactions
        WHERE tenant_id = $1 AND product_id = $2 AND location_id = $3
        ORDER BY created_at DESC
        LIMIT $4
    )";
    
    auto result = conn->executeParams(query, {tenantId, productId, locationId, std::to_string(limit)});
    
    if (!result.isSuccess()) {
        LOG_ERROR("[InventoryRepository] Query failed: " + result.getError());
        return transactions;
    }
    
    LOG_DEBUG("[InventoryRepository] Found " + std::to_string(result.rowCount()) + " transactions");
    
    for (int i = 0; i < result.rowCount(); i++) {
        transactions.push_back(mapToTransaction(result, i));
    }
    
    return transactions;
}

std::vector<InventoryTransaction> InventoryRepositoryImpl::findByTenant(
    const std::string& tenantId,
    int limit
) {
    LOG_DEBUG("[InventoryRepository] findByTenant() - tenant: " + tenantId);
    
    std::vector<InventoryTransaction> transactions;
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, product_id, location_id, type,
               quantity, quantity_before, quantity_after,
               reference_type, reference_id, created_by, created_at
        FROM inventory_transactions
        WHERE tenant_id = $1
        ORDER BY created_at DESC
        LIMIT $2
    )";
    
    auto result = conn->executeParams(query, {tenantId, std::to_string(limit)});
    
    if (!result.isSuccess()) {
        LOG_ERROR("[InventoryRepository] Query failed: " + result.getError());
        return transactions;
    }
    
    LOG_DEBUG("[InventoryRepository] Found " + std::to_string(result.rowCount()) + " transactions");
    
    for (int i = 0; i < result.rowCount(); i++) {
        transactions.push_back(mapToTransaction(result, i));
    }
    
    return transactions;
}

int InventoryRepositoryImpl::getCurrentStock(
    const std::string& tenantId,
    const std::string& productId
) {
    LOG_DEBUG("[InventoryRepository] getCurrentStock() - product: " + productId);
    
    auto conn = pool_->acquire();
    
    // Pegar o último registro para saber o estoque atual
    std::string query = R"(
        SELECT quantity_after
        FROM inventory_transactions
        WHERE tenant_id = $1 AND product_id = $2
        ORDER BY created_at DESC
        LIMIT 1
    )";
    
    auto result = conn->executeParams(query, {tenantId, productId});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        LOG_DEBUG("[InventoryRepository] No transactions found, stock = 0");
        return 0;
    }
    
    int stock = std::stoi(result.getValue(0, 0));
    LOG_DEBUG("[InventoryRepository] Current stock: " + std::to_string(stock));
    return stock;
}

int InventoryRepositoryImpl::getCurrentStockByLocation(
    const std::string& tenantId,
    const std::string& productId,
    const std::string& locationId
) {
    LOG_DEBUG("[InventoryRepository] getCurrentStockByLocation() - product: " + productId + ", location: " + locationId);
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT quantity_after
        FROM inventory_transactions
        WHERE tenant_id = $1 AND product_id = $2 AND location_id = $3
        ORDER BY created_at DESC
        LIMIT 1
    )";
    
    auto result = conn->executeParams(query, {tenantId, productId, locationId});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        LOG_DEBUG("[InventoryRepository] No transactions found for location, stock = 0");
        return 0;
    }
    
    int stock = std::stoi(result.getValue(0, 0));
    LOG_DEBUG("[InventoryRepository] Current stock at location: " + std::to_string(stock));
    return stock;
}

} // namespace Inventory
} // namespace Domain

