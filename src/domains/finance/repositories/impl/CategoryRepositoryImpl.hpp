#pragma once
#include <memory>
#include "../CategoryRepository.hpp"
#include "../../../../core/database/ConnectionPool.hpp"

namespace Finance {

class CategoryRepositoryImpl : public CategoryRepository {
public:
    explicit CategoryRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);

    bool save(const Category& category) override;
    bool update(const Category& category) override;
    bool remove(const std::string& id) override;
    
    std::optional<Category> findById(const std::string& id, const std::string& tenantId) override;
    std::vector<Category> findByTenant(const std::string& tenantId) override;
    std::vector<Category> findByType(const std::string& type, const std::string& tenantId) override;
    std::vector<Category> findRootCategories(const std::string& tenantId) override;
    std::vector<Category> findSubcategories(const std::string& parentId, const std::string& tenantId) override;
    std::vector<Category> findActive(const std::string& tenantId) override;
    std::vector<Category> findSystemCategories() override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    Category mapToCategory(const Core::Database::QueryResult& result, int row);
};

} // namespace Finance

