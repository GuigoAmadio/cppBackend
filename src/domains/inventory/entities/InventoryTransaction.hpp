#pragma once

#include <string>
#include <optional>
#include "../value_objects/TransactionType.hpp"
#include "../value_objects/ReferenceType.hpp"

namespace Domain {
namespace Inventory {

/**
 * @brief Entity: InventoryTransaction
 * Representa uma transação de inventário (entrada, saída, ajuste, transferência)
 */
class InventoryTransaction {
public:
    InventoryTransaction(
        const std::string& id,
        const std::string& tenantId,
        const std::string& productId,
        const TransactionType& type,
        int quantity,
        int quantityBefore,
        int quantityAfter,
        const std::string& createdBy
    ) : id_(id),
        tenantId_(tenantId),
        productId_(productId),
        type_(type),
        quantity_(quantity),
        quantityBefore_(quantityBefore),
        quantityAfter_(quantityAfter),
        createdBy_(createdBy) {
        
        if (quantity == 0) {
            throw std::invalid_argument("Quantity cannot be zero");
        }
    }

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTenantId() const { return tenantId_; }
    const std::string& getProductId() const { return productId_; }
    const TransactionType& getType() const { return type_; }
    int getQuantity() const { return quantity_; }
    int getQuantityBefore() const { return quantityBefore_; }
    int getQuantityAfter() const { return quantityAfter_; }
    const std::string& getCreatedBy() const { return createdBy_; }
    
    const std::optional<std::string>& getLocationId() const { return locationId_; }
    const std::optional<ReferenceType>& getReferenceType() const { return referenceType_; }
    const std::optional<std::string>& getReferenceId() const { return referenceId_; }
    const std::optional<std::string>& getNotes() const { return notes_; }
    const std::optional<std::string>& getCreatedAt() const { return createdAt_; }

    // Setters
    void setLocationId(const std::string& locationId) { locationId_ = locationId; }
    void setReferenceType(const ReferenceType& type) { referenceType_ = type; }
    void setReferenceId(const std::string& refId) { referenceId_ = refId; }
    void setNotes(const std::string& notes) { notes_ = notes; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }

    // Business logic
    bool isInbound() const { 
        return type_.value() == TransactionType::Type::INBOUND; 
    }
    
    bool isOutbound() const { 
        return type_.value() == TransactionType::Type::OUTBOUND; 
    }
    
    bool isAdjustment() const { 
        return type_.value() == TransactionType::Type::ADJUSTMENT; 
    }
    
    bool isTransfer() const { 
        return type_.value() == TransactionType::Type::TRANSFER; 
    }

private:
    std::string id_;
    std::string tenantId_;
    std::string productId_;
    TransactionType type_;
    int quantity_;              // Quantidade movimentada
    int quantityBefore_;        // Estoque antes
    int quantityAfter_;         // Estoque depois
    std::string createdBy_;
    
    // Opcionais
    std::optional<std::string> locationId_;
    std::optional<ReferenceType> referenceType_;
    std::optional<std::string> referenceId_;
    std::optional<std::string> notes_;
    std::optional<std::string> createdAt_;
};

} // namespace Inventory
} // namespace Domain

