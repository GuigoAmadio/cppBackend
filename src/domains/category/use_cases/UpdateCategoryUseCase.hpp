#pragma once

#include <memory>
#include <optional>
#include "../repositories/CategoryRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Category {

struct UpdateCategoryDto {
    std::string categoryId;
    std::string tenantId;
    
    std::optional<std::string> name;
    std::optional<std::string> slug;
    std::optional<std::string> description;
    std::optional<std::string> imageUrl;
    std::optional<int> displayOrder;
    std::optional<std::string> status;
    std::optional<std::string> parentId;
};

struct UpdateCategoryResult {
    bool success;
    std::string message;
};

class UpdateCategoryUseCase {
public:
    explicit UpdateCategoryUseCase(std::shared_ptr<CategoryRepository> repository)
        : repository_(repository) {}

    UpdateCategoryResult execute(const UpdateCategoryDto& dto) {
        try {
            LOG_DEBUG("[UpdateCategoryUseCase] Starting - category_id: " + dto.categoryId);
            
            auto categoryOpt = repository_->findById(dto.categoryId);
            if (!categoryOpt.has_value()) {
                LOG_WARNING("[UpdateCategoryUseCase] Category not found");
                return {false, "Category not found"};
            }
            
            Category category = *categoryOpt;
            
            if (!dto.tenantId.empty() && category.getTenantId() != dto.tenantId) {
                LOG_WARNING("[UpdateCategoryUseCase] Access denied");
                return {false, "Access denied"};
            }
            
            if (dto.name.has_value()) {
                LOG_DEBUG("[UpdateCategoryUseCase] Updating name to: " + *dto.name);
                category.setName(*dto.name);
            }
            if (dto.slug.has_value()) {
                LOG_DEBUG("[UpdateCategoryUseCase] Updating slug to: " + *dto.slug);
                // Verificar slug duplicado
                auto existing = repository_->findBySlug(category.getTenantId(), *dto.slug);
                if (existing.has_value() && existing->getId() != dto.categoryId) {
                    LOG_WARNING("[UpdateCategoryUseCase] Slug already exists");
                    return {false, "Slug already exists"};
                }
                category.setSlug(*dto.slug);
            }
            if (dto.description.has_value()) {
                category.setDescription(*dto.description);
            }
            if (dto.imageUrl.has_value()) {
                category.setImageUrl(*dto.imageUrl);
            }
            if (dto.displayOrder.has_value()) {
                category.setDisplayOrder(*dto.displayOrder);
            }
            if (dto.status.has_value()) {
                category.setStatus(CategoryStatus(*dto.status));
            }
            if (dto.parentId.has_value()) {
                category.setParentId(*dto.parentId);
            }
            
            if (!repository_->update(category)) {
                return {false, "Failed to update category"};
            }
            
            return {true, "Category updated successfully"};
            
        } catch (const std::exception& e) {
            return {false, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<CategoryRepository> repository_;
};

} // namespace Category
} // namespace Domain

