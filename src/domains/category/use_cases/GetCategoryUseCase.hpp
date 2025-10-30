#pragma once

#include <memory>
#include "../repositories/CategoryRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Category {

struct GetCategoryDto {
    std::string categoryId;
    std::string tenantId;
};

struct GetCategoryResult {
    bool success;
    std::optional<Category> category;
    std::string message;
};

class GetCategoryUseCase {
public:
    explicit GetCategoryUseCase(std::shared_ptr<CategoryRepository> repository)
        : repository_(repository) {}

    GetCategoryResult execute(const GetCategoryDto& dto) {
        try {
            auto categoryOpt = repository_->findById(dto.categoryId);
            
            if (!categoryOpt.has_value()) {
                return {false, std::nullopt, "Category not found"};
            }
            
            if (!dto.tenantId.empty() && categoryOpt->getTenantId() != dto.tenantId) {
                return {false, std::nullopt, "Access denied"};
            }
            
            return {true, categoryOpt, "Category retrieved successfully"};
            
        } catch (const std::exception& e) {
            return {false, std::nullopt, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<CategoryRepository> repository_;
};

} // namespace Category
} // namespace Domain

