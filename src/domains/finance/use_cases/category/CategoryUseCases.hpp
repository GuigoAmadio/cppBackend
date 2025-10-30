#pragma once
#include <memory>
#include <vector>
#include <optional>
#include "../../repositories/CategoryRepository.hpp"
#include "../../entities/Category.hpp"
#include "../../../../core/utils/Logger.hpp"
#include "../../../../core/utils/IdGenerator.hpp"

namespace Finance {
namespace Utils = Core::Utils;

// ===== CREATE CATEGORY =====
struct CreateCategoryDTO {
    std::string tenantId;
    std::string workspaceId;
    std::string name;
    std::string type;  // income or expense
    std::string parentId;
    std::string color;
    std::string icon;
};

class CreateCategoryUseCase {
public:
    explicit CreateCategoryUseCase(std::shared_ptr<CategoryRepository> categoryRepo)
        : categoryRepo_(categoryRepo) {}

    std::string execute(const CreateCategoryDTO& dto) {
        if (dto.tenantId.empty() || dto.name.empty() || dto.type.empty()) {
            throw std::invalid_argument("Tenant ID, name, and type are required");
        }
        if (dto.type != "income" && dto.type != "expense") {
            throw std::invalid_argument("Type must be 'income' or 'expense'");
        }

        std::string id = Core::Utils::IdGenerator::generate("cat");
        Category category(id, dto.tenantId, dto.name, dto.type, true, false);
        
        if (!dto.workspaceId.empty()) category.setWorkspaceId(dto.workspaceId);
        if (!dto.parentId.empty()) category.setParentId(dto.parentId);
        if (!dto.color.empty()) category.setColor(dto.color);
        if (!dto.icon.empty()) category.setIcon(dto.icon);

        if (!categoryRepo_->save(category)) {
            throw std::runtime_error("Failed to save category");
        }

        return id;
    }

private:
    std::shared_ptr<CategoryRepository> categoryRepo_;
};

// ===== GET CATEGORY =====
class GetCategoryUseCase {
public:
    explicit GetCategoryUseCase(std::shared_ptr<CategoryRepository> categoryRepo)
        : categoryRepo_(categoryRepo) {}

    Category execute(const std::string& categoryId, const std::string& tenantId) {
        auto catOpt = categoryRepo_->findById(categoryId, tenantId);
        if (!catOpt.has_value()) throw std::runtime_error("Category not found");
        return catOpt.value();
    }

private:
    std::shared_ptr<CategoryRepository> categoryRepo_;
};

// ===== LIST CATEGORIES =====
class ListCategoriesUseCase {
public:
    explicit ListCategoriesUseCase(std::shared_ptr<CategoryRepository> categoryRepo)
        : categoryRepo_(categoryRepo) {}

    std::vector<Category> execute(const std::string& tenantId, const std::string& type = "", bool rootOnly = false) {
        if (rootOnly) {
            return categoryRepo_->findRootCategories(tenantId);
        } else if (!type.empty()) {
            return categoryRepo_->findByType(type, tenantId);
        } else {
            return categoryRepo_->findByTenant(tenantId);
        }
    }

private:
    std::shared_ptr<CategoryRepository> categoryRepo_;
};

// ===== UPDATE CATEGORY =====
struct UpdateCategoryDTO {
    std::string categoryId;
    std::string tenantId;
    std::optional<std::string> name;
    std::optional<std::string> color;
    std::optional<std::string> icon;
    std::optional<bool> isActive;
};

class UpdateCategoryUseCase {
public:
    explicit UpdateCategoryUseCase(std::shared_ptr<CategoryRepository> categoryRepo)
        : categoryRepo_(categoryRepo) {}

    bool execute(const UpdateCategoryDTO& dto) {
        Utils::Logger::info("[UpdateCategoryUseCase] Starting - categoryId: " + dto.categoryId + ", tenantId: " + dto.tenantId);
        
        auto catOpt = categoryRepo_->findById(dto.categoryId, dto.tenantId);
        if (!catOpt.has_value()) {
            Utils::Logger::info("[UpdateCategoryUseCase] ❌ Category not found!");
            throw std::runtime_error("Category not found");
        }

        Category cat = catOpt.value();
        Utils::Logger::info("[UpdateCategoryUseCase] Category found: " + cat.getName() + ", isSystem: " + std::string(cat.isSystem() ? "TRUE" : "FALSE"));
        
        if (cat.isSystem()) {
            Utils::Logger::info("[UpdateCategoryUseCase] ❌ Cannot modify system category!");
            throw std::runtime_error("Cannot modify system categories");
        }

        if (dto.name.has_value()) {
            Utils::Logger::info("[UpdateCategoryUseCase] Updating name to: " + *dto.name);
            cat.setName(*dto.name);
        }
        if (dto.color.has_value()) {
            Utils::Logger::info("[UpdateCategoryUseCase] Updating color to: " + *dto.color);
            cat.setColor(*dto.color);
        }
        if (dto.icon.has_value()) cat.setIcon(*dto.icon);
        if (dto.isActive.has_value()) cat.setActive(*dto.isActive);

        Utils::Logger::info("[UpdateCategoryUseCase] About to call categoryRepo_->update...");
        bool result = categoryRepo_->update(cat);
        Utils::Logger::info("[UpdateCategoryUseCase] Repository update() returned: " + std::string(result ? "TRUE" : "FALSE"));
        return result;
    }

private:
    std::shared_ptr<CategoryRepository> categoryRepo_;
};

// ===== DELETE CATEGORY =====
class DeleteCategoryUseCase {
public:
    explicit DeleteCategoryUseCase(std::shared_ptr<CategoryRepository> categoryRepo)
        : categoryRepo_(categoryRepo) {}

    bool execute(const std::string& categoryId, const std::string& tenantId) {
        Utils::Logger::info("[DeleteCategoryUseCase] Starting - categoryId: " + categoryId + ", tenantId: " + tenantId);
        
        auto catOpt = categoryRepo_->findById(categoryId, tenantId);
        if (!catOpt.has_value()) {
            Utils::Logger::info("[DeleteCategoryUseCase] ❌ Category not found!");
            throw std::runtime_error("Category not found");
        }

        auto cat = catOpt.value();
        Utils::Logger::info("[DeleteCategoryUseCase] Category found: " + cat.getName() + ", isSystem: " + std::string(cat.isSystem() ? "TRUE" : "FALSE"));
        
        if (cat.isSystem()) {
            Utils::Logger::info("[DeleteCategoryUseCase] ❌ Cannot delete system category!");
            throw std::runtime_error("Cannot delete system categories");
        }

        // Check for subcategories
        Utils::Logger::info("[DeleteCategoryUseCase] Checking for subcategories...");
        auto subcats = categoryRepo_->findSubcategories(categoryId, tenantId);
        Utils::Logger::info("[DeleteCategoryUseCase] Found " + std::to_string(subcats.size()) + " subcategories");
        if (!subcats.empty()) {
            Utils::Logger::info("[DeleteCategoryUseCase] ❌ Cannot delete category with subcategories!");
            throw std::runtime_error("Cannot delete category with subcategories");
        }

        Utils::Logger::info("[DeleteCategoryUseCase] About to call categoryRepo_->remove...");
        bool result = categoryRepo_->remove(categoryId);
        Utils::Logger::info("[DeleteCategoryUseCase] Repository remove() returned: " + std::string(result ? "TRUE" : "FALSE"));
        return result;
    }

private:
    std::shared_ptr<CategoryRepository> categoryRepo_;
};

} // namespace Finance

