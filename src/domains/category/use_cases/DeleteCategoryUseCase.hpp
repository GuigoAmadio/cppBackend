#pragma once

#include <memory>
#include "../repositories/CategoryRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Category {

struct DeleteCategoryDto {
    std::string categoryId;
    std::string tenantId;
};

struct DeleteCategoryResult {
    bool success;
    std::string message;
};

class DeleteCategoryUseCase {
public:
    explicit DeleteCategoryUseCase(std::shared_ptr<CategoryRepository> repository)
        : repository_(repository) {}

    DeleteCategoryResult execute(const DeleteCategoryDto& dto) {
        try {
            auto categoryOpt = repository_->findById(dto.categoryId);
            if (!categoryOpt.has_value()) {
                return {false, "Category not found"};
            }
            
            if (!dto.tenantId.empty() && categoryOpt->getTenantId() != dto.tenantId) {
                return {false, "Access denied"};
            }
            
            // Verificar se tem filhos
            auto children = repository_->findByParent(dto.tenantId, dto.categoryId);
            if (!children.empty()) {
                return {false, "Cannot delete category with children"};
            }
            
            if (!repository_->remove(dto.categoryId)) {
                return {false, "Failed to delete category"};
            }
            
            return {true, "Category deleted successfully"};
            
        } catch (const std::exception& e) {
            return {false, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<CategoryRepository> repository_;
};

} // namespace Category
} // namespace Domain

