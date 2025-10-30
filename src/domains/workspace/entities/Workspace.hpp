#pragma once

#include <string>
#include <optional>
#include "../value_objects/WorkspaceType.hpp"

namespace Domain {
namespace Workspace {

/**
 * @brief Entity: Workspace
 * Representa um workspace (espaço de trabalho) dentro de um tenant
 */
class Workspace {
public:
    Workspace(
        const std::string& id,
        const std::string& tenantId,
        const std::string& name,
        const std::string& slug,
        const WorkspaceType& type,
        const std::string& createdBy
    ) : id_(id),
        tenantId_(tenantId),
        name_(name),
        slug_(slug),
        type_(type),
        createdBy_(createdBy),
        isActive_(true) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getName() const { return name_; }
    const std::string& getSlug() const { return slug_; }
    const WorkspaceType& getType() const { return type_; }
    const std::optional<std::string>& getDescription() const { return description_; }
    const std::string& getSettings() const { return settings_; }
    bool isActive() const { return isActive_; }
    const std::string& getCreatedBy() const { return createdBy_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setName(const std::string& name) { name_ = name; }
    void setDescription(const std::string& description) { description_ = description; }
    void setSettings(const std::string& settings) { settings_ = settings; }
    void setActive(bool active) { isActive_ = active; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }

    // Business logic
    bool canBeDeleted() const {
        return isActive_; // Adicione lógicas mais complexas aqui
    }

private:
    std::string id_;
    std::string tenantId_;
    std::string name_;
    std::string slug_;
    WorkspaceType type_;
    std::optional<std::string> description_;
    std::string settings_ = "{}"; // JSON settings
    bool isActive_;
    std::string createdBy_;
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
};

} // namespace Workspace
} // namespace Domain

