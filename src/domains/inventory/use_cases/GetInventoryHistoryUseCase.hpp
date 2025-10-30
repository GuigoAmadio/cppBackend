#pragma once

#include <memory>
#include <vector>
#include "../repositories/InventoryRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Inventory {

struct GetInventoryHistoryDto {
    std::string tenantId;
    std::string productId;     // Opcional: se vazio, busca por tenant
    std::string locationId;    // Opcional: filtrar por localização
    int limit = 100;
};

struct GetInventoryHistoryResult {
    bool success;
    std::vector<InventoryTransaction> transactions;
    std::string message;
};

class GetInventoryHistoryUseCase {
public:
    explicit GetInventoryHistoryUseCase(std::shared_ptr<InventoryRepository> repository)
        : repository_(repository) {}

    GetInventoryHistoryResult execute(const GetInventoryHistoryDto& dto) {
        try {
            LOG_DEBUG("[GetInventoryHistoryUseCase] Starting execution");
            LOG_DEBUG("[GetInventoryHistoryUseCase] tenant: " + dto.tenantId);
            LOG_DEBUG("[GetInventoryHistoryUseCase] product: " + dto.productId);
            LOG_DEBUG("[GetInventoryHistoryUseCase] location: " + dto.locationId);
            LOG_DEBUG("[GetInventoryHistoryUseCase] limit: " + std::to_string(dto.limit));
            
            // Validar tenant ID
            if (dto.tenantId.empty()) {
                LOG_ERROR("[GetInventoryHistoryUseCase] Tenant ID is empty");
                return {false, {}, "Tenant ID is required"};
            }
            
            std::vector<InventoryTransaction> transactions;
            
            // Buscar por product + location
            if (!dto.productId.empty() && !dto.locationId.empty()) {
                LOG_DEBUG("[GetInventoryHistoryUseCase] Searching by product AND location");
                transactions = repository_->findByProductAndLocation(
                    dto.tenantId, dto.productId, dto.locationId, dto.limit
                );
            }
            // Buscar por product
            else if (!dto.productId.empty()) {
                LOG_DEBUG("[GetInventoryHistoryUseCase] Searching by product only");
                transactions = repository_->findByProduct(
                    dto.tenantId, dto.productId, dto.limit
                );
            }
            // Buscar por tenant
            else {
                LOG_DEBUG("[GetInventoryHistoryUseCase] Searching by tenant only");
                transactions = repository_->findByTenant(dto.tenantId, dto.limit);
            }
            
            LOG_DEBUG("[GetInventoryHistoryUseCase] Found " + std::to_string(transactions.size()) + " transactions");
            return {true, transactions, "History retrieved successfully"};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[GetInventoryHistoryUseCase] Exception: " + std::string(e.what()));
            return {false, {}, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<InventoryRepository> repository_;
};

} // namespace Inventory
} // namespace Domain

