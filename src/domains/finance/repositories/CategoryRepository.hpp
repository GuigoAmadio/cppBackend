#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "../entities/Category.hpp"

namespace Finance {

class CategoryRepository {
public:
    virtual ~CategoryRepository() = default;

    virtual bool save(const Category& category) = 0;
    virtual bool update(const Category& category) = 0;
    virtual bool remove(const std::string& id) = 0;
    
    virtual std::optional<Category> findById(const std::string& id, const std::string& tenantId) = 0;
    virtual std::vector<Category> findByTenant(const std::string& tenantId) = 0;
    virtual std::vector<Category> findByType(const std::string& type, const std::string& tenantId) = 0;
    virtual std::vector<Category> findRootCategories(const std::string& tenantId) = 0;
    virtual std::vector<Category> findSubcategories(const std::string& parentId, const std::string& tenantId) = 0;
    virtual std::vector<Category> findActive(const std::string& tenantId) = 0;
    virtual std::vector<Category> findSystemCategories() = 0;
};

} // namespace Finance

