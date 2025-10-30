#pragma once

#include <memory>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include "../repositories/InventoryRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Inventory {

// Helper inline para gerar UUID simples
inline std::string generateSimpleUUID() {
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

struct RecordTransactionDto {
    std::string tenantId;
    std::string productId;
    std::string type;          // "in", "out", "adjustment", "transfer"
    int quantity;
    std::string createdBy;
    
    // Opcionais
    std::string locationId;
    std::string referenceType; // "order", "purchase", "return", etc
    std::string referenceId;
    std::string notes;
};

struct RecordTransactionResult {
    bool success;
    std::string transactionId;
    std::string message;
    int newStock;              // Estoque após a transação
};

class RecordTransactionUseCase {
public:
    explicit RecordTransactionUseCase(std::shared_ptr<InventoryRepository> repository)
        : repository_(repository) {}

    RecordTransactionResult execute(const RecordTransactionDto& dto) {
        try {
            LOG_DEBUG("[RecordTransactionUseCase] Starting execution");
            LOG_DEBUG("[RecordTransactionUseCase] tenant: " + dto.tenantId);
            LOG_DEBUG("[RecordTransactionUseCase] product: " + dto.productId);
            LOG_DEBUG("[RecordTransactionUseCase] type: " + dto.type);
            LOG_DEBUG("[RecordTransactionUseCase] quantity: " + std::to_string(dto.quantity));
            
            // 1. Validar campos obrigatórios
            if (dto.tenantId.empty()) {
                LOG_ERROR("[RecordTransactionUseCase] Tenant ID is empty");
                return {false, "", "Tenant ID is required", 0};
            }
            if (dto.productId.empty()) {
                LOG_ERROR("[RecordTransactionUseCase] Product ID is empty");
                return {false, "", "Product ID is required", 0};
            }
            if (dto.type.empty()) {
                LOG_ERROR("[RecordTransactionUseCase] Type is empty");
                return {false, "", "Transaction type is required", 0};
            }
            if (dto.quantity == 0) {
                LOG_ERROR("[RecordTransactionUseCase] Quantity is zero");
                return {false, "", "Quantity cannot be zero", 0};
            }
            if (dto.createdBy.empty()) {
                LOG_ERROR("[RecordTransactionUseCase] CreatedBy is empty");
                return {false, "", "Created by user ID is required", 0};
            }
            
            LOG_DEBUG("[RecordTransactionUseCase] All required fields validated");
            
            // 2. Obter estoque atual
            LOG_DEBUG("[RecordTransactionUseCase] Getting current stock...");
            int currentStock = 0;
            if (!dto.locationId.empty()) {
                currentStock = repository_->getCurrentStockByLocation(
                    dto.tenantId, dto.productId, dto.locationId
                );
            } else {
                currentStock = repository_->getCurrentStock(dto.tenantId, dto.productId);
            }
            LOG_DEBUG("[RecordTransactionUseCase] Current stock: " + std::to_string(currentStock));
            
            // 3. Calcular novo estoque
            int newStock = currentStock;
            TransactionType transactionType(dto.type);
            
            if (transactionType.value() == TransactionType::Type::INBOUND) {
                newStock += dto.quantity;
                LOG_DEBUG("[RecordTransactionUseCase] INBOUND transaction, adding quantity");
            } else if (transactionType.value() == TransactionType::Type::OUTBOUND) {
                newStock -= dto.quantity;
                LOG_DEBUG("[RecordTransactionUseCase] OUTBOUND transaction, subtracting quantity");
            } else if (transactionType.value() == TransactionType::Type::ADJUSTMENT) {
                // Para ajuste, a quantidade é o valor absoluto (não incremental)
                newStock = dto.quantity;
                LOG_DEBUG("[RecordTransactionUseCase] ADJUSTMENT transaction, setting absolute quantity");
            } else if (transactionType.value() == TransactionType::Type::TRANSFER) {
                // Para transferência, subtrair da origem
                newStock -= dto.quantity;
                LOG_DEBUG("[RecordTransactionUseCase] TRANSFER transaction, subtracting from source");
            }
            
            // 4. Validar estoque não pode ficar negativo
            if (newStock < 0) {
                LOG_WARNING("[RecordTransactionUseCase] Insufficient stock - current: " + 
                    std::to_string(currentStock) + ", requested: " + std::to_string(dto.quantity));
                return {false, "", "Insufficient stock", 0};
            }
            
            LOG_DEBUG("[RecordTransactionUseCase] New stock calculated: " + std::to_string(newStock));
            
            // 5. Criar transação
            std::string transactionId = generateSimpleUUID();
            LOG_DEBUG("[RecordTransactionUseCase] Generated transaction ID: " + transactionId);
            
            InventoryTransaction transaction(
                transactionId,
                dto.tenantId,
                dto.productId,
                transactionType,
                dto.quantity,
                currentStock,
                newStock,
                dto.createdBy
            );
            
            // Campos opcionais
            if (!dto.locationId.empty()) {
                transaction.setLocationId(dto.locationId);
            }
            if (!dto.referenceType.empty()) {
                transaction.setReferenceType(ReferenceType(dto.referenceType));
            }
            if (!dto.referenceId.empty()) {
                transaction.setReferenceId(dto.referenceId);
            }
            if (!dto.notes.empty()) {
                transaction.setNotes(dto.notes);
            }
            
            // 6. Salvar
            LOG_DEBUG("[RecordTransactionUseCase] Calling repository save()");
            bool saved = repository_->save(transaction);
            
            if (!saved) {
                LOG_ERROR("[RecordTransactionUseCase] Failed to save transaction");
                return {false, "", "Failed to save transaction", 0};
            }
            
            LOG_DEBUG("[RecordTransactionUseCase] Transaction recorded successfully");
            return {true, transactionId, "Transaction recorded successfully", newStock};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[RecordTransactionUseCase] Exception: " + std::string(e.what()));
            return {false, "", std::string("Error: ") + e.what(), 0};
        }
    }

private:
    std::shared_ptr<InventoryRepository> repository_;
};

} // namespace Inventory
} // namespace Domain

