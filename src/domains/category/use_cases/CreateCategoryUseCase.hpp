#pragma once

#include <memory>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include "../repositories/CategoryRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Category {

inline std::string generateCategoryUUID() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;
    
    uint64_t part1 = dis(gen);
    uint64_t part2 = dis(gen);
    
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8) << (part1 >> 32)
        << "-" << std::setw(4) << ((part1 >> 16) & 0xFFFF)
        << "-" << std::setw(4) << (part1 & 0xFFFF)
        << "-" << std::setw(4) << (part2 >> 48)
        << "-" << std::setw(12) << (part2 & 0xFFFFFFFFFFFF);
    
    return oss.str();
}

struct CreateCategoryDto {
    std::string tenantId;
    std::string name;
    std::string slug;
    std::string parentId;      // Opcional
    std::string description;   // Opcional
    std::string imageUrl;      // Opcional
    int displayOrder = 0;
    std::string createdBy;
};

struct CreateCategoryResult {
    bool success;
    std::string categoryId;
    std::string message;
};

class CreateCategoryUseCase {
public:
    explicit CreateCategoryUseCase(std::shared_ptr<CategoryRepository> repository)
        : repository_(repository) {}

    CreateCategoryResult execute(const CreateCategoryDto& dto) {
        try {
            LOG_DEBUG("[CreateCategoryUseCase] Starting execution");
            
            if (dto.tenantId.empty() || dto.name.empty() || dto.slug.empty() || dto.createdBy.empty()) {
                return {false, "", "Required fields missing"};
            }
            
            // Verificar slug duplicado
            auto existing = repository_->findBySlug(dto.tenantId, dto.slug);
            if (existing.has_value()) {
                return {false, "", "Slug already exists"};
            }
            
            // Verificar se parent existe (se fornecido)
            if (!dto.parentId.empty()) {
                auto parent = repository_->findById(dto.parentId);
                if (!parent.has_value()) {
                    return {false, "", "Parent category not found"};
                }
            }
            
            std::string categoryId = generateCategoryUUID();
            Category category(categoryId, dto.tenantId, dto.name, dto.slug, dto.createdBy);
            
            if (!dto.parentId.empty()) category.setParentId(dto.parentId);
            if (!dto.description.empty()) category.setDescription(dto.description);
            if (!dto.imageUrl.empty()) category.setImageUrl(dto.imageUrl);
            category.setDisplayOrder(dto.displayOrder);
            
            if (!repository_->save(category)) {
                return {false, "", "Failed to save category"};
            }
            
            LOG_DEBUG("[CreateCategoryUseCase] Category created: " + categoryId);
            return {true, categoryId, "Category created successfully"};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[CreateCategoryUseCase] Exception: " + std::string(e.what()));
            return {false, "", std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<CategoryRepository> repository_;
};

} // namespace Category
} // namespace Domain

