#pragma once
#include <string>
#include <optional>
#include "../value_objects/TransactionType.hpp"

namespace Finance {

class Category {
public:
    Category(
        const std::string& id,
        const std::string& tenantId,
        const std::string& name,
        const std::string& type, // income or expense
        bool isActive,
        bool isSystem = false
    ) : id_(id),
        tenantId_(tenantId),
        name_(name),
        type_(type),
        isActive_(isActive),
        isSystem_(isSystem) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getWorkspaceId() const { return workspaceId_; }
    const std::string& getName() const { return name_; }
    const std::string& getType() const { return type_; }
    const std::optional<std::string>& getParentId() const { return parentId_; }
    const std::optional<std::string>& getColor() const { return color_; }
    const std::optional<std::string>& getIcon() const { return icon_; }
    bool isActive() const { return isActive_; }
    bool isSystem() const { return isSystem_; }

    // Setters
    void setWorkspaceId(const std::string& workspaceId) { workspaceId_ = workspaceId; }
    void setName(const std::string& name) {
        if (name.empty()) {
            throw std::invalid_argument("Category name cannot be empty");
        }
        name_ = name;
    }
    void setParentId(const std::string& parentId) { parentId_ = parentId; }
    void setColor(const std::string& color) { color_ = color; }
    void setIcon(const std::string& icon) { icon_ = icon; }
    void setActive(bool isActive) { isActive_ = isActive; }

    // Business logic
    bool isIncome() const { return type_ == "income"; }
    bool isExpense() const { return type_ == "expense"; }
    bool isSubcategory() const { return parentId_.has_value() && !parentId_->empty(); }

    void activate() { isActive_ = true; }
    void deactivate() {
        if (isSystem_) {
            throw std::runtime_error("Cannot deactivate system category");
        }
        isActive_ = false;
    }

private:
    std::string id_;
    std::string tenantId_;
    std::string workspaceId_;
    std::string name_;
    std::string type_; // income or expense
    std::optional<std::string> parentId_;
    std::optional<std::string> color_;
    std::optional<std::string> icon_;
    bool isActive_;
    bool isSystem_; // System categories cannot be deleted
};

} // namespace Finance

