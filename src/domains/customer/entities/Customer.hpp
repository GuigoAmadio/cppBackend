#pragma once

#include <string>
#include <optional>
#include <vector>
#include "../value_objects/CustomerType.hpp"
#include "../value_objects/CustomerStatus.hpp"

namespace Domain {
namespace Customer {

/**
 * @brief Entity: Customer
 * Representa um cliente (pessoa física ou jurídica)
 */
class Customer {
public:
    Customer(
        const std::string& id,
        const std::string& tenantId,
        const std::string& name,
        const CustomerType& type,
        const std::string& createdBy
    ) : id_(id),
        tenantId_(tenantId),
        name_(name),
        type_(type),
        status_(CustomerStatus::Status::ACTIVE),
        createdBy_(createdBy) {
        
        if (name.empty() || name.length() > 500) {
            throw std::invalid_argument("Customer name must be 1-500 characters");
        }
    }

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getName() const { return name_; }
    const CustomerType& getType() const { return type_; }
    const CustomerStatus& getStatus() const { return status_; }
    const std::string& getCreatedBy() const { return createdBy_; }

    const std::optional<std::string>& getWorkspaceId() const { return workspaceId_; }
    const std::optional<std::string>& getEmail() const { return email_; }
    const std::optional<std::string>& getPhone() const { return phone_; }
    const std::optional<std::string>& getDocument() const { return document_; }  // CPF/CNPJ
    const std::optional<std::string>& getAddress() const { return address_; }     // JSON
    const std::optional<std::string>& getNotes() const { return notes_; }
    const std::vector<std::string>& getTags() const { return tags_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setName(const std::string& name) {
        if (name.empty() || name.length() > 500) {
            throw std::invalid_argument("Customer name must be 1-500 characters");
        }
        name_ = name;
    }
    
    void setWorkspaceId(const std::string& workspaceId) { workspaceId_ = workspaceId; }
    void setEmail(const std::string& email) { email_ = email; }
    void setPhone(const std::string& phone) { phone_ = phone; }
    void setDocument(const std::string& document) { document_ = document; }
    void setAddress(const std::string& address) { address_ = address; }
    void setNotes(const std::string& notes) { notes_ = notes; }
    void setTags(const std::vector<std::string>& tags) { tags_ = tags; }
    void setStatus(const CustomerStatus& status) { status_ = status; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }

    // Business logic
    void activate() { status_ = CustomerStatus(CustomerStatus::Status::ACTIVE); }
    void deactivate() { status_ = CustomerStatus(CustomerStatus::Status::INACTIVE); }
    void block() { status_ = CustomerStatus(CustomerStatus::Status::BLOCKED); }
    void markAsProspect() { status_ = CustomerStatus(CustomerStatus::Status::PROSPECT); }
    
    bool isActive() const { return status_.isActive(); }
    bool isBlocked() const { return status_.isBlocked(); }
    
    bool hasEmail() const { return email_.has_value() && !email_->empty(); }
    bool hasPhone() const { return phone_.has_value() && !phone_->empty(); }
    bool hasDocument() const { return document_.has_value() && !document_->empty(); }

private:
    std::string id_;
    std::string tenantId_;
    std::string name_;
    CustomerType type_;
    CustomerStatus status_;
    std::string createdBy_;
    
    // Opcionais
    std::optional<std::string> workspaceId_;
    std::optional<std::string> email_;
    std::optional<std::string> phone_;
    std::optional<std::string> document_;      // CPF/CNPJ
    std::optional<std::string> address_;       // JSON: {street, city, state, zip, country}
    std::optional<std::string> notes_;
    std::vector<std::string> tags_;
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
};

} // namespace Customer
} // namespace Domain

