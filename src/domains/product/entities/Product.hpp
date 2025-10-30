#pragma once

#include <string>
#include <optional>
#include <vector>
#include "../value_objects/ProductCode.hpp"
#include "../value_objects/ProductType.hpp"
#include "../value_objects/ProductStatus.hpp"
#include "../value_objects/Price.hpp"
#include "../value_objects/Stock.hpp"

namespace Domain {
namespace Product {

/**
 * @brief Entity: Product
 * Representa um produto no sistema
 */
class Product {
public:
    // Construtor para novos produtos
    Product(
        const std::string& id,
        const std::string& tenantId,
        const ProductCode& code,
        const std::string& name,
        const ProductType& type,
        const Price& price,
        const std::string& createdBy
    ) : id_(id),
        tenantId_(tenantId),
        code_(code),
        name_(name),
        type_(type),
        status_(ProductStatus::Status::ACTIVE),
        price_(price),
        cost_(Price(0)),
        stock_(Stock()),
        trackInventory_(true),
        createdBy_(createdBy) {
        
        if (name.empty() || name.length() > 500) {
            throw std::invalid_argument("Product name must be 1-500 characters");
        }
    }

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const ProductCode& getCode() const { return code_; }
    const std::string& getName() const { return name_; }
    const ProductType& getType() const { return type_; }
    const ProductStatus& getStatus() const { return status_; }
    const Price& getPrice() const { return price_; }
    const Price& getCost() const { return cost_; }
    const Stock& getStock() const { return stock_; }
    bool trackInventory() const { return trackInventory_; }
    const std::string& getCreatedBy() const { return createdBy_; }

    const std::optional<std::string>& getWorkspaceId() const { return workspaceId_; }
    const std::optional<std::string>& getCategoryId() const { return categoryId_; }
    const std::optional<std::string>& getDescription() const { return description_; }
    const std::optional<std::string>& getSku() const { return sku_; }
    const std::optional<std::string>& getBarcode() const { return barcode_; }
    const std::optional<double>& getWeight() const { return weight_; }
    const std::optional<std::string>& getWeightUnit() const { return weightUnit_; }
    const std::optional<std::string>& getDimensions() const { return dimensions_; }
    const std::optional<std::string>& getMetadata() const { return metadata_; }
    const std::vector<std::string>& getTags() const { return tags_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setName(const std::string& name) {
        if (name.empty() || name.length() > 500) {
            throw std::invalid_argument("Product name must be 1-500 characters");
        }
        name_ = name;
    }
    void setWorkspaceId(const std::string& workspaceId) { workspaceId_ = workspaceId; }
    void setCategoryId(const std::string& categoryId) { categoryId_ = categoryId; }
    void setDescription(const std::string& description) { description_ = description; }
    void setStatus(const ProductStatus& status) { status_ = status; }
    void setPrice(const Price& price) { price_ = price; }
    void setCost(const Price& cost) { cost_ = cost; }
    void setStock(const Stock& stock) { stock_ = stock; }
    void setTrackInventory(bool track) { trackInventory_ = track; }
    void setSku(const std::string& sku) { sku_ = sku; }
    void setBarcode(const std::string& barcode) { barcode_ = barcode; }
    void setWeight(double weight, const std::string& unit = "kg") {
        if (weight < 0) {
            throw std::invalid_argument("Weight cannot be negative");
        }
        weight_ = weight;
        weightUnit_ = unit;
    }
    void setDimensions(const std::string& dimensions) { dimensions_ = dimensions; }
    void setMetadata(const std::string& metadata) { metadata_ = metadata; }
    void setTags(const std::vector<std::string>& tags) { tags_ = tags; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }

    // Business logic methods
    void activate() { status_ = ProductStatus(ProductStatus::Status::ACTIVE); }
    void deactivate() { status_ = ProductStatus(ProductStatus::Status::INACTIVE); }
    void archive() { status_ = ProductStatus(ProductStatus::Status::ARCHIVED); }

    bool isAvailable() const { 
        return status_.isActive() && (!trackInventory_ || stock_.isAvailable());
    }

    bool isLowStock() const {
        return status_.isActive() && trackInventory_ && stock_.isLowStock();
    }

    bool canBeSold(int requestedQuantity = 1) const {
        if (!status_.isActive()) return false;
        if (!trackInventory_) return true;
        return stock_.quantity() >= requestedQuantity;
    }

    void addStock(int quantity) {
        if (!trackInventory_) {
            throw std::logic_error("Cannot add stock to product with inventory tracking disabled");
        }
        stock_.add(quantity);
    }

    void subtractStock(int quantity) {
        if (!trackInventory_) {
            throw std::logic_error("Cannot subtract stock from product with inventory tracking disabled");
        }
        stock_.subtract(quantity);
    }

private:
    // Identificadores
    std::string id_;
    std::string tenantId_;
    std::optional<std::string> workspaceId_;
    std::optional<std::string> categoryId_;

    // Informações básicas
    ProductCode code_;
    std::string name_;
    std::optional<std::string> description_;
    ProductType type_;
    ProductStatus status_;

    // Pricing
    Price price_;
    Price cost_;

    // Stock
    bool trackInventory_;
    Stock stock_;

    // Identificadores alternativos
    std::optional<std::string> sku_;
    std::optional<std::string> barcode_;

    // Atributos físicos
    std::optional<double> weight_;
    std::optional<std::string> weightUnit_;
    std::optional<std::string> dimensions_; // JSON string

    // Metadata
    std::optional<std::string> metadata_; // JSON string
    std::vector<std::string> tags_;

    // Timestamps
    std::string createdBy_;
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
};

} // namespace Product
} // namespace Domain

