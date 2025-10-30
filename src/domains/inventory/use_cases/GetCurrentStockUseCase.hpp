#pragma once

#include <memory>
#include "../repositories/InventoryRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Inventory {

struct GetCurrentStockDto {
    std::string tenantId;
    std::string productId;
    std::string locationId;  // Opcional
};

struct GetCurrentStockResult {
    bool success;
    int stock;
    std::string message;
};

class GetCurrentStockUseCase {
public:
    explicit GetCurrentStockUseCase(std::shared_ptr<InventoryRepository> repository)
        : repository_(repository) {}

    GetCurrentStockResult execute(const GetCurrentStockDto& dto) {
        try {
            LOG_DEBUG("[GetCurrentStockUseCase] Starting execution");
            LOG_DEBUG("[GetCurrentStockUseCase] tenant: " + dto.tenantId);
            LOG_DEBUG("[GetCurrentStockUseCase] product: " + dto.productId);
            LOG_DEBUG("[GetCurrentStockUseCase] location: " + dto.locationId);
            
            if (dto.tenantId.empty()) {
                LOG_ERROR("[GetCurrentStockUseCase] Tenant ID is empty");
                return {false, 0, "Tenant ID is required"};
            }
            
            if (dto.productId.empty()) {
                LOG_ERROR("[GetCurrentStockUseCase] Product ID is empty");
                return {false, 0, "Product ID is required"};
            }
            
            int stock = 0;
            if (!dto.locationId.empty()) {
                LOG_DEBUG("[GetCurrentStockUseCase] Getting stock by location");
                stock = repository_->getCurrentStockByLocation(
                    dto.tenantId, dto.productId, dto.locationId
                );
            } else {
                LOG_DEBUG("[GetCurrentStockUseCase] Getting total stock");
                stock = repository_->getCurrentStock(dto.tenantId, dto.productId);
            }
            
            LOG_DEBUG("[GetCurrentStockUseCase] Current stock: " + std::to_string(stock));
            return {true, stock, "Stock retrieved successfully"};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[GetCurrentStockUseCase] Exception: " + std::string(e.what()));
            return {false, 0, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<InventoryRepository> repository_;
};

} // namespace Inventory
} // namespace Domain

