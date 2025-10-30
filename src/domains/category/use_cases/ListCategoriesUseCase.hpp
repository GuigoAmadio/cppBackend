#pragma once

#include <memory>
#include <vector>
#include <optional>
#include "../repositories/CategoryRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Category {

struct ListCategoriesDto {
    std::string tenantId;
    std::optional<std::string> parentId;  // nullopt = root categories
    int limit = 100;
    int offset = 0;
};

struct ListCategoriesResult {
    bool success;
    std::vector<Category> categories;
    int total;
    std::string message;
};

class ListCategoriesUseCase {
public:
    explicit ListCategoriesUseCase(std::shared_ptr<CategoryRepository> repository)
        : repository_(repository) {}

    ListCategoriesResult execute(const ListCategoriesDto& dto) {
        try {
            if (dto.tenantId.empty()) {
                return {false, {}, 0, "Tenant ID is required"};
            }
            
            std::vector<Category> categories;
            
            if (dto.parentId.has_value()) {
                // Buscar filhos de uma categoria específica
                categories = repository_->findByParent(dto.tenantId, dto.parentId);
            } else if (dto.limit == 100 && dto.offset == 0) {
                // Buscar categorias raiz (sem filtro de paginação)
                categories = repository_->findRootCategories(dto.tenantId);
            } else {
                // Buscar todas com paginação
                categories = repository_->findByTenant(dto.tenantId, dto.limit, dto.offset);
            }
            
            return {true, categories, static_cast<int>(categories.size()), "Categories retrieved successfully"};
            
        } catch (const std::exception& e) {
            return {false, {}, 0, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<CategoryRepository> repository_;
};

} // namespace Category
} // namespace Domain

