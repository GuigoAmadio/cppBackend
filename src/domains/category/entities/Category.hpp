#pragma once

#include <string>
#include <optional>
#include "../value_objects/CategoryStatus.hpp"

namespace Domain {
namespace Category {

/**
 * @brief Entity: Category
 * Categoria de produtos com suporte a hierarquia (parent-child)
 */
class Category {
public:
    Category(
        const std::string& id,
        const std::string& tenantId,
        const std::string& name,
        const std::string& slug,
        const std::string& createdBy
    ) : id_(id),
        tenantId_(tenantId),
        name_(name),
        slug_(slug),
        status_(CategoryStatus::Status::ACTIVE),
        createdBy_(createdBy) {
        
        if (name.empty() || name.length() > 200) {
            throw std::invalid_argument("Category name must be 1-200 characters");
        }
        
        if (slug.empty() || slug.length() > 200) {
            throw std::invalid_argument("Category slug must be 1-200 characters");
        }
    }

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getName() const { return name_; }
    const std::string& getSlug() const { return slug_; }
    const CategoryStatus& getStatus() const { return status_; }
    const std::string& getCreatedBy() const { return createdBy_; }
    
    const std::optional<std::string>& getParentId() const { return parentId_; }
    const std::optional<std::string>& getDescription() const { return description_; }
    const std::optional<std::string>& getImageUrl() const { return imageUrl_; }
    const std::optional<int>& getDisplayOrder() const { return displayOrder_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setName(const std::string& name) {
        if (name.empty() || name.length() > 200) {
            throw std::invalid_argument("Category name must be 1-200 characters");
        }
        name_ = name;
    }
    
    void setSlug(const std::string& slug) {
        if (slug.empty() || slug.length() > 200) {
            throw std::invalid_argument("Category slug must be 1-200 characters");
        }
        slug_ = slug;
    }
    
    void setStatus(const CategoryStatus& status) { status_ = status; }
    void setParentId(const std::string& parentId) { parentId_ = parentId; }
    void setDescription(const std::string& description) { description_ = description; }
    void setImageUrl(const std::string& imageUrl) { imageUrl_ = imageUrl; }
    void setDisplayOrder(int order) { displayOrder_ = order; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }

    // Business logic
    void activate() { status_ = CategoryStatus(CategoryStatus::Status::ACTIVE); }
    void deactivate() { status_ = CategoryStatus(CategoryStatus::Status::INACTIVE); }
    void archive() { status_ = CategoryStatus(CategoryStatus::Status::ARCHIVED); }
    
    bool isActive() const { return status_.isActive(); }
    bool hasParent() const { return parentId_.has_value(); }
    bool isRootCategory() const { return !parentId_.has_value(); }

private:
    std::string id_;
    std::string tenantId_;
    std::string name_;
    std::string slug_;
    CategoryStatus status_;
    std::string createdBy_;
    
    // Opcionais
    std::optional<std::string> parentId_;      // Categoria pai (para hierarquia)
    std::optional<std::string> description_;
    std::optional<std::string> imageUrl_;
    std::optional<int> displayOrder_;          // Ordem de exibição
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
};

} // namespace Category
} // namespace Domain

