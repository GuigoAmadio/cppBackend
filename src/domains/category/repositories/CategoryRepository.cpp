#include "CategoryRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Category {

Category CategoryRepositoryImpl::mapToCategory(
    const Core::Database::QueryResult& result,
    int row
) {
    LOG_DEBUG("[CategoryRepository] Mapping row " + std::to_string(row) + " to Category");
    
    std::string id = result.getValue(row, 0);
    std::string tenantId = result.getValue(row, 1);
    std::string name = result.getValue(row, 3);
    std::string slug = result.getValue(row, 4);
    std::string createdBy = result.getValue(row, 9);
    
    Category category(id, tenantId, name, slug, createdBy);
    
    // parent_id (col 2)
    std::string parentId = result.getValue(row, 2);
    if (!parentId.empty()) {
        category.setParentId(parentId);
    }
    
    // description (col 5)
    std::string description = result.getValue(row, 5);
    if (!description.empty()) {
        category.setDescription(description);
    }
    
    // image_url (col 6)
    std::string imageUrl = result.getValue(row, 6);
    if (!imageUrl.empty()) {
        category.setImageUrl(imageUrl);
    }
    
    // display_order (col 7)
    std::string displayOrderStr = result.getValue(row, 7);
    if (!displayOrderStr.empty()) {
        category.setDisplayOrder(std::stoi(displayOrderStr));
    }
    
    // status (col 8)
    std::string statusStr = result.getValue(row, 8);
    if (!statusStr.empty()) {
        category.setStatus(CategoryStatus(statusStr));
    }
    
    // created_at (col 10), updated_at (col 11)
    std::string createdAt = result.getValue(row, 10);
    if (!createdAt.empty()) {
        category.setCreatedAt(createdAt);
    }
    
    std::string updatedAt = result.getValue(row, 11);
    if (!updatedAt.empty()) {
        category.setUpdatedAt(updatedAt);
    }
    
    return category;
}

bool CategoryRepositoryImpl::save(const Category& category) {
    LOG_DEBUG("[CategoryRepository] save() called - category_id: " + category.getId());
    LOG_DEBUG("[CategoryRepository] tenant: " + category.getTenantId());
    LOG_DEBUG("[CategoryRepository] name: " + category.getName());
    LOG_DEBUG("[CategoryRepository] slug: " + category.getSlug());
    
    auto conn = pool_->acquire();
    LOG_DEBUG("[CategoryRepository] Connection acquired");
    
    std::string query = R"(
        INSERT INTO product_categories (
            id, tenant_id, parent_id, name, slug, description,
            image_url, display_order, status, created_by, created_at, updated_at
        ) VALUES (
            $1, $2, NULLIF($3,''), $4, $5, NULLIF($6,''),
            NULLIF($7,''), $8, $9, $10, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP
        )
    )";
    
    std::vector<std::string> params;
    params.push_back(category.getId());
    params.push_back(category.getTenantId());
    params.push_back(category.getParentId().value_or(""));
    params.push_back(category.getName());
    params.push_back(category.getSlug());
    params.push_back(category.getDescription().value_or(""));
    params.push_back(category.getImageUrl().value_or(""));
    params.push_back(category.getDisplayOrder().has_value() ? std::to_string(*category.getDisplayOrder()) : "0");
    params.push_back(category.getStatus().toString());
    params.push_back(category.getCreatedBy());
    
    LOG_DEBUG("[CategoryRepository] Parameters prepared, total: " + std::to_string(params.size()));
    LOG_DEBUG("[CategoryRepository] Executing INSERT...");
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        LOG_ERROR("[CategoryRepository] INSERT failed!");
        LOG_ERROR("[CategoryRepository] Error: " + result.getError());
        return false;
    }
    
    LOG_DEBUG("[CategoryRepository] Category saved successfully");
    return true;
}

bool CategoryRepositoryImpl::update(const Category& category) {
    LOG_DEBUG("[CategoryRepository] update() called - category_id: " + category.getId());
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE product_categories SET
            name = $2,
            slug = $3,
            description = NULLIF($4,''),
            image_url = NULLIF($5,''),
            display_order = $6,
            status = $7,
            parent_id = NULLIF($8,''),
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $1
    )";
    
    std::vector<std::string> params;
    params.push_back(category.getId());
    params.push_back(category.getName());
    params.push_back(category.getSlug());
    params.push_back(category.getDescription().value_or(""));
    params.push_back(category.getImageUrl().value_or(""));
    params.push_back(category.getDisplayOrder().has_value() ? std::to_string(*category.getDisplayOrder()) : "0");
    params.push_back(category.getStatus().toString());
    params.push_back(category.getParentId().value_or(""));
    
    LOG_DEBUG("[CategoryRepository] Executing UPDATE...");
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        LOG_ERROR("[CategoryRepository] UPDATE failed: " + result.getError());
        return false;
    }
    
    LOG_DEBUG("[CategoryRepository] Category updated successfully");
    return true;
}

bool CategoryRepositoryImpl::remove(const std::string& id) {
    LOG_DEBUG("[CategoryRepository] remove() called - category_id: " + id);
    
    auto conn = pool_->acquire();
    
    // Soft delete: marcar como archived
    std::string query = R"(
        UPDATE product_categories SET
            status = 'archived',
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $1
    )";
    
    auto result = conn->executeParams(query, {id});
    
    if (!result.isSuccess()) {
        LOG_ERROR("[CategoryRepository] Soft delete failed: " + result.getError());
        return false;
    }
    
    LOG_DEBUG("[CategoryRepository] Category archived");
    return true;
}

std::optional<Category> CategoryRepositoryImpl::findById(const std::string& id) {
    LOG_DEBUG("[CategoryRepository] findById() called - id: " + id);
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, parent_id, name, slug, description,
               image_url, display_order, status, created_by, created_at, updated_at
        FROM product_categories
        WHERE id = $1
    )";
    
    auto result = conn->executeParams(query, {id});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        LOG_DEBUG("[CategoryRepository] Category not found");
        return std::nullopt;
    }
    
    LOG_DEBUG("[CategoryRepository] Category found");
    return mapToCategory(result, 0);
}

std::optional<Category> CategoryRepositoryImpl::findBySlug(
    const std::string& tenantId,
    const std::string& slug
) {
    LOG_DEBUG("[CategoryRepository] findBySlug() - slug: " + slug);
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, parent_id, name, slug, description,
               image_url, display_order, status, created_by, created_at, updated_at
        FROM product_categories
        WHERE tenant_id = $1 AND slug = $2
    )";
    
    auto result = conn->executeParams(query, {tenantId, slug});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        return std::nullopt;
    }
    
    return mapToCategory(result, 0);
}

std::vector<Category> CategoryRepositoryImpl::findByTenant(
    const std::string& tenantId,
    int limit,
    int offset
) {
    LOG_DEBUG("[CategoryRepository] findByTenant() - tenant: " + tenantId);
    
    std::vector<Category> categories;
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, parent_id, name, slug, description,
               image_url, display_order, status, created_by, created_at, updated_at
        FROM product_categories
        WHERE tenant_id = $1
        ORDER BY display_order, name
        LIMIT $2 OFFSET $3
    )";
    
    auto result = conn->executeParams(query, {
        tenantId, 
        std::to_string(limit), 
        std::to_string(offset)
    });
    
    if (!result.isSuccess()) {
        LOG_ERROR("[CategoryRepository] Query failed: " + result.getError());
        return categories;
    }
    
    LOG_DEBUG("[CategoryRepository] Found " + std::to_string(result.rowCount()) + " categories");
    
    for (int i = 0; i < result.rowCount(); i++) {
        categories.push_back(mapToCategory(result, i));
    }
    
    return categories;
}

std::vector<Category> CategoryRepositoryImpl::findByParent(
    const std::string& tenantId,
    const std::optional<std::string>& parentId
) {
    LOG_DEBUG("[CategoryRepository] findByParent() - parent: " + (parentId.has_value() ? *parentId : "ROOT"));
    
    std::vector<Category> categories;
    auto conn = pool_->acquire();
    
    std::string query;
    std::vector<std::string> params;
    params.push_back(tenantId);
    
    if (parentId.has_value()) {
        // Buscar filhos de um parent específico (apenas ativos)
        query = R"(
            SELECT id, tenant_id, parent_id, name, slug, description,
                   image_url, display_order, status, created_by, created_at, updated_at
            FROM product_categories
            WHERE tenant_id = $1 AND parent_id = $2 AND status != 'archived'
            ORDER BY display_order, name
        )";
        params.push_back(*parentId);
    } else {
        // Buscar categorias raiz (sem parent, apenas ativos)
        query = R"(
            SELECT id, tenant_id, parent_id, name, slug, description,
                   image_url, display_order, status, created_by, created_at, updated_at
            FROM product_categories
            WHERE tenant_id = $1 AND parent_id IS NULL AND status != 'archived'
            ORDER BY display_order, name
        )";
    }
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        LOG_ERROR("[CategoryRepository] Query failed: " + result.getError());
        return categories;
    }
    
    LOG_DEBUG("[CategoryRepository] Found " + std::to_string(result.rowCount()) + " children");
    
    for (int i = 0; i < result.rowCount(); i++) {
        categories.push_back(mapToCategory(result, i));
    }
    
    return categories;
}

std::vector<Category> CategoryRepositoryImpl::findRootCategories(const std::string& tenantId) {
    return findByParent(tenantId, std::nullopt);
}

} // namespace Category
} // namespace Domain

