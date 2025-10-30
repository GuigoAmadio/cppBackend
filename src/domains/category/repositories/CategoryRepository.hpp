#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>
#include "../entities/Category.hpp"
#include "../../../core/database/ConnectionPool.hpp"

namespace Domain {
namespace Category {

/**
 * @brief Interface: CategoryRepository
 */
class CategoryRepository {
public:
    virtual ~CategoryRepository() = default;

    virtual bool save(const Category& category) = 0;
    virtual bool update(const Category& category) = 0;
    virtual bool remove(const std::string& id) = 0;
    
    virtual std::optional<Category> findById(const std::string& id) = 0;
    virtual std::optional<Category> findBySlug(const std::string& tenantId, const std::string& slug) = 0;
    
    virtual std::vector<Category> findByTenant(
        const std::string& tenantId,
        int limit = 100,
        int offset = 0
    ) = 0;
    
    virtual std::vector<Category> findByParent(
        const std::string& tenantId,
        const std::optional<std::string>& parentId  // nullopt = root categories
    ) = 0;
    
    virtual std::vector<Category> findRootCategories(const std::string& tenantId) = 0;
};

/**
 * @brief Implementação PostgreSQL
 */
class CategoryRepositoryImpl : public CategoryRepository {
public:
    explicit CategoryRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
        : pool_(pool) {}

    bool save(const Category& category) override;
    bool update(const Category& category) override;
    bool remove(const std::string& id) override;
    std::optional<Category> findById(const std::string& id) override;
    std::optional<Category> findBySlug(const std::string& tenantId, const std::string& slug) override;
    std::vector<Category> findByTenant(const std::string& tenantId, int limit, int offset) override;
    std::vector<Category> findByParent(const std::string& tenantId, const std::optional<std::string>& parentId) override;
    std::vector<Category> findRootCategories(const std::string& tenantId) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    Category mapToCategory(const Core::Database::QueryResult& result, int row);
};

// Factory
inline std::shared_ptr<CategoryRepository> createCategoryRepository(
    std::shared_ptr<Core::Database::ConnectionPool> pool
) {
    return std::make_shared<CategoryRepositoryImpl>(pool);
}

} // namespace Category
} // namespace Domain

