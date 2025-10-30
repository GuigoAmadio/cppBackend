#include "ProductRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>

namespace Domain {
namespace Product {

// Factory implementation
std::shared_ptr<ProductRepository> createProductRepository(
    std::shared_ptr<Core::Database::ConnectionPool> pool
) {
    return std::make_shared<ProductRepositoryImpl>(pool);
}

// Helper para converter QueryResult em Product
Product ProductRepositoryImpl::resultToProduct(const Core::Database::QueryResult& result, int row) {
    // Campos obrigatórios
    std::string id = result.getValue(row, 0);
    std::string tenantId = result.getValue(row, 1);
    std::string code = result.getValue(row, 3);
    std::string name = result.getValue(row, 4);
    std::string typeStr = result.getValue(row, 6);
    std::string statusStr = result.getValue(row, 7);
    
    // Price (em decimal, converter para cents)
    double priceDecimal = std::stod(result.getValue(row, 8));
    std::string currency = result.getValue(row, 10);
    Price price = Price::fromDecimal(priceDecimal, currency);
    
    std::string createdBy = result.getValue(row, 18);
    
    // Criar Product
    Product product(
        id,
        tenantId,
        ProductCode(code),
        name,
        ProductType(typeStr),
        price,
        createdBy
    );
    
    // Campos opcionais
    if (!result.isNull(row, 2)) {
        product.setWorkspaceId(result.getValue(row, 2));
    }
    
    if (!result.isNull(row, 5)) {
        product.setDescription(result.getValue(row, 5));
    }
    
    // Status
    product.setStatus(ProductStatus(statusStr));
    
    // Cost
    if (!result.isNull(row, 9)) {
        double costDecimal = std::stod(result.getValue(row, 9));
        product.setCost(Price::fromDecimal(costDecimal, currency));
    }
    
    // Inventory tracking
    bool trackInventory = (result.getValue(row, 11) == "t" || result.getValue(row, 11) == "true");
    product.setTrackInventory(trackInventory);
    
    // Stock
    int stockQty = std::stoi(result.getValue(row, 12));
    int lowStockThreshold = std::stoi(result.getValue(row, 13));
    product.setStock(Stock(stockQty, lowStockThreshold));
    
    // SKU, Barcode
    if (!result.isNull(row, 14)) {
        product.setSku(result.getValue(row, 14));
    }
    if (!result.isNull(row, 15)) {
        product.setBarcode(result.getValue(row, 15));
    }
    
    // Weight
    if (!result.isNull(row, 16)) {
        double weight = std::stod(result.getValue(row, 16));
        std::string weightUnit = result.getValue(row, 17);
        product.setWeight(weight, weightUnit);
    }
    
    // Timestamps
    if (!result.isNull(row, 19)) {
        product.setCreatedAt(result.getValue(row, 19));
    }
    if (!result.isNull(row, 20)) {
        product.setUpdatedAt(result.getValue(row, 20));
    }
    
    return product;
}

bool ProductRepositoryImpl::save(const Product& product) {
    LOG_DEBUG("[ProductRepository] save() called");
    LOG_DEBUG("[ProductRepository] Product ID: " + product.getId());
    LOG_DEBUG("[ProductRepository] Tenant ID: " + product.getTenantId());
    LOG_DEBUG("[ProductRepository] Code: " + product.getCode().toString());
    LOG_DEBUG("[ProductRepository] Name: " + product.getName());
    
    auto conn = pool_->acquire();
    LOG_DEBUG("[ProductRepository] Connection acquired");
    
    std::string query = R"(
        INSERT INTO products (
            id, tenant_id, workspace_id, category_id, code, name, description,
            type, status, price, cost, currency, track_inventory, stock_quantity,
            low_stock_threshold, sku, barcode, weight, weight_unit, created_by,
            created_at, updated_at
        ) VALUES (
            NULLIF($1,''), NULLIF($2,''), NULLIF($3,''), NULLIF($4,''), $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15,
            $16, $17, NULLIF($18,'')::numeric, $19, $20, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP
        )
    )";
    
    std::vector<std::string> params;
    params.push_back(product.getId());
    params.push_back(product.getTenantId());
    params.push_back(product.getWorkspaceId().has_value() ? *product.getWorkspaceId() : "");  // NULL if empty
    params.push_back(product.getCategoryId().has_value() ? *product.getCategoryId() : "");    // NULL if empty
    params.push_back(product.getCode().toString());
    params.push_back(product.getName());
    params.push_back(product.getDescription().value_or(""));
    params.push_back(product.getType().toString());
    params.push_back(product.getStatus().toString());
    params.push_back(product.getPrice().toString());
    params.push_back(product.getCost().toString());
    params.push_back(product.getPrice().currency());
    params.push_back(product.trackInventory() ? "true" : "false");
    params.push_back(std::to_string(product.getStock().quantity()));
    params.push_back(std::to_string(product.getStock().lowStockThreshold()));
    params.push_back(product.getSku().value_or(""));
    params.push_back(product.getBarcode().value_or(""));
    params.push_back(product.getWeight().has_value() ? std::to_string(*product.getWeight()) : "");
    params.push_back(product.getWeightUnit().value_or("kg"));
    params.push_back(product.getCreatedBy());
    
    LOG_DEBUG("[ProductRepository] Parameters prepared, total: " + std::to_string(params.size()));
    LOG_DEBUG("[ProductRepository] Executing query...");
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        LOG_ERROR("[ProductRepository] Query failed!");
        LOG_ERROR("[ProductRepository] Error: " + result.getError());
        LOG_DEBUG("[ProductRepository] Query was: " + query);
        LOG_DEBUG("[ProductRepository] Param 1 (id): " + params[0]);
        LOG_DEBUG("[ProductRepository] Param 2 (tenant_id): " + params[1]);
        LOG_DEBUG("[ProductRepository] Param 3 (workspace_id): " + (params[2].empty() ? "NULL" : params[2]));
        LOG_DEBUG("[ProductRepository] Param 4 (category_id): " + (params[3].empty() ? "NULL" : params[3]));
        LOG_DEBUG("[ProductRepository] Param 5 (code): " + params[4]);
    } else {
        LOG_DEBUG("[ProductRepository] Query executed successfully");
    }
    
    return result.isSuccess();
}

bool ProductRepositoryImpl::update(const Product& product) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE products SET
            name = $1, description = $2, type = $3, status = $4,
            price = $5, cost = $6, currency = $7,
            track_inventory = $8, stock_quantity = $9, low_stock_threshold = $10,
            sku = $11, barcode = $12, weight = $13, weight_unit = $14,
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $15
    )";
    
    std::vector<std::string> params = {
        product.getName(),
        product.getDescription().value_or(""),
        product.getType().toString(),
        product.getStatus().toString(),
        product.getPrice().toString(),
        product.getCost().toString(),
        product.getPrice().currency(),
        product.trackInventory() ? "true" : "false",
        std::to_string(product.getStock().quantity()),
        std::to_string(product.getStock().lowStockThreshold()),
        product.getSku().value_or(""),
        product.getBarcode().value_or(""),
        product.getWeight().has_value() ? std::to_string(*product.getWeight()) : "",
        product.getWeightUnit().value_or("kg"),
        product.getId()
    };
    
    auto result = conn->executeParams(query, params);
    return result.isSuccess();
}

bool ProductRepositoryImpl::remove(const std::string& id) {
    auto conn = pool_->acquire();
    std::string query = "DELETE FROM products WHERE id = $1";
    auto result = conn->executeParams(query, {id});
    return result.isSuccess();
}

std::optional<Product> ProductRepositoryImpl::findById(const std::string& id) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, code, name, description,
               type, status, price, cost, currency,
               track_inventory, stock_quantity, low_stock_threshold,
               sku, barcode, weight, weight_unit, created_by,
               created_at, updated_at
        FROM products
        WHERE id = $1
    )";
    
    auto result = conn->executeParams(query, {id});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        return std::nullopt;
    }
    
    return resultToProduct(result, 0);
}

std::optional<Product> ProductRepositoryImpl::findByCode(const std::string& tenantId, const std::string& code) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, code, name, description,
               type, status, price, cost, currency,
               track_inventory, stock_quantity, low_stock_threshold,
               sku, barcode, weight, weight_unit, created_by,
               created_at, updated_at
        FROM products
        WHERE tenant_id = $1 AND code = $2
    )";
    
    auto result = conn->executeParams(query, {tenantId, code});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        return std::nullopt;
    }
    
    return resultToProduct(result, 0);
}

std::optional<Product> ProductRepositoryImpl::findBySku(const std::string& tenantId, const std::string& sku) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, code, name, description,
               type, status, price, cost, currency,
               track_inventory, stock_quantity, low_stock_threshold,
               sku, barcode, weight, weight_unit, created_by,
               created_at, updated_at
        FROM products
        WHERE tenant_id = $1 AND sku = $2
    )";
    
    auto result = conn->executeParams(query, {tenantId, sku});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        return std::nullopt;
    }
    
    return resultToProduct(result, 0);
}

std::vector<Product> ProductRepositoryImpl::findByTenant(const std::string& tenantId, int limit, int offset) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, code, name, description,
               type, status, price, cost, currency,
               track_inventory, stock_quantity, low_stock_threshold,
               sku, barcode, weight, weight_unit, created_by,
               created_at, updated_at
        FROM products
        WHERE tenant_id = $1
        ORDER BY created_at DESC
        LIMIT $2 OFFSET $3
    )";
    
    auto result = conn->executeParams(query, {tenantId, std::to_string(limit), std::to_string(offset)});
    
    std::vector<Product> products;
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); ++i) {
            products.push_back(resultToProduct(result, i));
        }
    }
    
    return products;
}

std::vector<Product> ProductRepositoryImpl::findByWorkspace(const std::string& workspaceId, int limit, int offset) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, code, name, description,
               type, status, price, cost, currency,
               track_inventory, stock_quantity, low_stock_threshold,
               sku, barcode, weight, weight_unit, created_by,
               created_at, updated_at
        FROM products
        WHERE workspace_id = $1
        ORDER BY created_at DESC
        LIMIT $2 OFFSET $3
    )";
    
    auto result = conn->executeParams(query, {workspaceId, std::to_string(limit), std::to_string(offset)});
    
    std::vector<Product> products;
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); ++i) {
            products.push_back(resultToProduct(result, i));
        }
    }
    
    return products;
}

std::vector<Product> ProductRepositoryImpl::findByCategory(const std::string& categoryId, int limit, int offset) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, code, name, description,
               type, status, price, cost, currency,
               track_inventory, stock_quantity, low_stock_threshold,
               sku, barcode, weight, weight_unit, created_by,
               created_at, updated_at
        FROM products
        WHERE category_id = $1
        ORDER BY created_at DESC
        LIMIT $2 OFFSET $3
    )";
    
    auto result = conn->executeParams(query, {categoryId, std::to_string(limit), std::to_string(offset)});
    
    std::vector<Product> products;
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); ++i) {
            products.push_back(resultToProduct(result, i));
        }
    }
    
    return products;
}

std::vector<Product> ProductRepositoryImpl::findByStatus(const std::string& tenantId, const std::string& status, int limit, int offset) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, code, name, description,
               type, status, price, cost, currency,
               track_inventory, stock_quantity, low_stock_threshold,
               sku, barcode, weight, weight_unit, created_by,
               created_at, updated_at
        FROM products
        WHERE tenant_id = $1 AND status = $2
        ORDER BY created_at DESC
        LIMIT $3 OFFSET $4
    )";
    
    auto result = conn->executeParams(query, {tenantId, status, std::to_string(limit), std::to_string(offset)});
    
    std::vector<Product> products;
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); ++i) {
            products.push_back(resultToProduct(result, i));
        }
    }
    
    return products;
}

std::vector<Product> ProductRepositoryImpl::search(const std::string& tenantId, const std::string& query, int limit) {
    auto conn = pool_->acquire();
    
    std::string sql = R"(
        SELECT id, tenant_id, workspace_id, code, name, description,
               type, status, price, cost, currency,
               track_inventory, stock_quantity, low_stock_threshold,
               sku, barcode, weight, weight_unit, created_by,
               created_at, updated_at
        FROM products
        WHERE tenant_id = $1 
          AND (
              name ILIKE $2 
              OR code ILIKE $2 
              OR sku ILIKE $2 
              OR description ILIKE $2
          )
        ORDER BY created_at DESC
        LIMIT $3
    )";
    
    std::string searchPattern = "%" + query + "%";
    auto result = conn->executeParams(sql, {tenantId, searchPattern, std::to_string(limit)});
    
    std::vector<Product> products;
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); ++i) {
            products.push_back(resultToProduct(result, i));
        }
    }
    
    return products;
}

std::vector<Product> ProductRepositoryImpl::findLowStock(const std::string& tenantId, int limit) {
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, code, name, description,
               type, status, price, cost, currency,
               track_inventory, stock_quantity, low_stock_threshold,
               sku, barcode, weight, weight_unit, created_by,
               created_at, updated_at
        FROM products
        WHERE tenant_id = $1 
          AND track_inventory = true
          AND status = 'active'
          AND stock_quantity <= low_stock_threshold
          AND stock_quantity > 0
        ORDER BY stock_quantity ASC
        LIMIT $2
    )";
    
    auto result = conn->executeParams(query, {tenantId, std::to_string(limit)});
    
    std::vector<Product> products;
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); ++i) {
            products.push_back(resultToProduct(result, i));
        }
    }
    
    return products;
}

int ProductRepositoryImpl::countByTenant(const std::string& tenantId) {
    auto conn = pool_->acquire();
    std::string query = "SELECT COUNT(*) FROM products WHERE tenant_id = $1";
    auto result = conn->executeParams(query, {tenantId});
    
    if (result.isSuccess() && result.rowCount() > 0) {
        return std::stoi(result.getValue(0, 0));
    }
    
    return 0;
}

int ProductRepositoryImpl::countByWorkspace(const std::string& workspaceId) {
    auto conn = pool_->acquire();
    std::string query = "SELECT COUNT(*) FROM products WHERE workspace_id = $1";
    auto result = conn->executeParams(query, {workspaceId});
    
    if (result.isSuccess() && result.rowCount() > 0) {
        return std::stoi(result.getValue(0, 0));
    }
    
    return 0;
}

} // namespace Product
} // namespace Domain

