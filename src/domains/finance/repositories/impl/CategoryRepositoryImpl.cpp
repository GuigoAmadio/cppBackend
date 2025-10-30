#include "CategoryRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Finance {

namespace Utils = Core::Utils;

CategoryRepositoryImpl::CategoryRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool) : pool_(pool) {}

bool CategoryRepositoryImpl::save(const Category& cat) {
    auto conn = pool_->acquire();
    std::string query = R"(
        INSERT INTO transaction_categories (id, tenant_id, workspace_id, name, type, parent_id, color, icon, is_active, is_system)
        VALUES ($1, $2, NULLIF($3,''), $4, $5, NULLIF($6,''), NULLIF($7,''), NULLIF($8,''), $9, $10)
    )";
    std::vector<std::string> params = {cat.getId(), cat.getTenantId(), cat.getWorkspaceId(), cat.getName(), cat.getType(), cat.getParentId().value_or(""), cat.getColor().value_or(""), cat.getIcon().value_or(""), cat.isActive() ? "true" : "false", cat.isSystem() ? "true" : "false"};
    return conn->executeParams(query, params).isSuccess();
}

bool CategoryRepositoryImpl::update(const Category& cat) {
    auto conn = pool_->acquire();
    std::string query = "UPDATE transaction_categories SET name = $3, color = NULLIF($4,''), icon = NULLIF($5,''), is_active = $6 WHERE id = $1 AND tenant_id = $2";
    std::vector<std::string> params = {cat.getId(), cat.getTenantId(), cat.getName(), cat.getColor().value_or(""), cat.getIcon().value_or(""), cat.isActive() ? "true" : "false"};
    return conn->executeParams(query, params).isSuccess();
}

bool CategoryRepositoryImpl::remove(const std::string& id) {
    auto conn = pool_->acquire();
    return conn->executeParams("DELETE FROM transaction_categories WHERE id = $1", {id}).isSuccess();
}

std::optional<Category> CategoryRepositoryImpl::findById(const std::string& id, const std::string& tenantId) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams("SELECT * FROM transaction_categories WHERE id = $1 AND (tenant_id = $2 OR tenant_id = 'system')", {id, tenantId});
    if (result.rowCount() == 0) return std::nullopt;
    return mapToCategory(result, 0);
}

std::vector<Category> CategoryRepositoryImpl::findByTenant(const std::string& tenantId) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams("SELECT * FROM transaction_categories WHERE tenant_id = $1 OR tenant_id = 'system' ORDER BY name", {tenantId});
    std::vector<Category> categories;
    for (int i = 0; i < result.rowCount(); i++) categories.push_back(mapToCategory(result, i));
    return categories;
}

std::vector<Category> CategoryRepositoryImpl::findByType(const std::string& type, const std::string& tenantId) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams("SELECT * FROM transaction_categories WHERE type = $1 AND (tenant_id = $2 OR tenant_id = 'system') ORDER BY name", {type, tenantId});
    std::vector<Category> categories;
    for (int i = 0; i < result.rowCount(); i++) categories.push_back(mapToCategory(result, i));
    return categories;
}

std::vector<Category> CategoryRepositoryImpl::findRootCategories(const std::string& tenantId) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams("SELECT * FROM transaction_categories WHERE parent_id IS NULL AND (tenant_id = $1 OR tenant_id = 'system') ORDER BY name", {tenantId});
    std::vector<Category> categories;
    for (int i = 0; i < result.rowCount(); i++) categories.push_back(mapToCategory(result, i));
    return categories;
}

std::vector<Category> CategoryRepositoryImpl::findSubcategories(const std::string& parentId, const std::string& tenantId) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams("SELECT * FROM transaction_categories WHERE parent_id = $1 AND (tenant_id = $2 OR tenant_id = 'system') ORDER BY name", {parentId, tenantId});
    std::vector<Category> categories;
    for (int i = 0; i < result.rowCount(); i++) categories.push_back(mapToCategory(result, i));
    return categories;
}

std::vector<Category> CategoryRepositoryImpl::findActive(const std::string& tenantId) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams("SELECT * FROM transaction_categories WHERE is_active = true AND (tenant_id = $1 OR tenant_id = 'system') ORDER BY name", {tenantId});
    std::vector<Category> categories;
    for (int i = 0; i < result.rowCount(); i++) categories.push_back(mapToCategory(result, i));
    return categories;
}

std::vector<Category> CategoryRepositoryImpl::findSystemCategories() {
    auto conn = pool_->acquire();
    auto result = conn->executeParams("SELECT * FROM transaction_categories WHERE is_system = true ORDER BY name", {});
    std::vector<Category> categories;
    for (int i = 0; i < result.rowCount(); i++) categories.push_back(mapToCategory(result, i));
    return categories;
}

Category CategoryRepositoryImpl::mapToCategory(const Core::Database::QueryResult& result, int row) {
    // id, tenant_id, workspace_id, name, type, parent_id, color, icon, is_system, is_active, created_at, updated_at
    Category cat(
        result.getValue(row, 0),  // id
        result.getValue(row, 1),  // tenant_id
        result.getValue(row, 3),  // name
        result.getValue(row, 4),  // type
        result.getValue(row, 9) == "t" || result.getValue(row, 9) == "true",  // is_active (CORRIGIDO: era 8)
        result.getValue(row, 8) == "t" || result.getValue(row, 8) == "true"   // is_system (CORRIGIDO: era 9)
    );
    cat.setWorkspaceId(result.getValue(row, 2));
    std::string parentId = result.getValue(row, 5);
    if (!parentId.empty()) cat.setParentId(parentId);
    std::string color = result.getValue(row, 6);
    if (!color.empty()) cat.setColor(color);
    std::string icon = result.getValue(row, 7);
    if (!icon.empty()) cat.setIcon(icon);
    return cat;
}

} // namespace Finance

