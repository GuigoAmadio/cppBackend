#pragma once

#include <memory>
#include <string>
#include <optional>
#include "../repositories/ProductRepository.hpp"

namespace Domain {
namespace Product {

struct UpdateProductDto {
    std::string productId;
    std::string tenantId; // Para validação
    
    // Campos atualizáveis (todos opcionais)
    std::optional<std::string> name;
    std::optional<std::string> description;
    std::optional<std::string> status; // "active", "inactive", "archived"
    std::optional<double> price;
    std::optional<double> cost;
    std::optional<std::string> currency;
    std::optional<int> stockQuantity;
    std::optional<int> lowStockThreshold;
    std::optional<bool> trackInventory;
    std::optional<std::string> sku;
    std::optional<std::string> barcode;
    std::optional<double> weight;
    std::optional<std::string> weightUnit;
};

struct UpdateProductResult {
    bool success;
    std::string message;
};

class UpdateProductUseCase {
public:
    explicit UpdateProductUseCase(std::shared_ptr<ProductRepository> repository)
        : repository_(repository) {}

    UpdateProductResult execute(const UpdateProductDto& dto) {
        try {
            // 1. Buscar produto existente
            auto productOpt = repository_->findById(dto.productId);
            
            if (!productOpt.has_value()) {
                return {false, "Product not found"};
            }

            Product product = *productOpt;

            // 2. Validar tenant ownership
            if (!dto.tenantId.empty() && product.getTenantId() != dto.tenantId) {
                return {false, "Access denied: product belongs to another tenant"};
            }

            // 3. Atualizar campos fornecidos
            if (dto.name.has_value()) {
                product.setName(*dto.name);  // ✅ AGORA ATUALIZA DE VERDADE!
            }

            if (dto.description.has_value()) {
                product.setDescription(*dto.description);
            }

            if (dto.status.has_value()) {
                product.setStatus(ProductStatus(*dto.status));
            }

            if (dto.price.has_value()) {
                std::string currency = dto.currency.value_or(product.getPrice().currency());
                product.setPrice(Price::fromDecimal(*dto.price, currency));
            }

            if (dto.cost.has_value()) {
                std::string currency = dto.currency.value_or(product.getPrice().currency());
                product.setCost(Price::fromDecimal(*dto.cost, currency));
            }

            if (dto.trackInventory.has_value()) {
                product.setTrackInventory(*dto.trackInventory);
            }

            if (dto.stockQuantity.has_value() || dto.lowStockThreshold.has_value()) {
                int newQty = dto.stockQuantity.value_or(product.getStock().quantity());
                int newThreshold = dto.lowStockThreshold.value_or(product.getStock().lowStockThreshold());
                product.setStock(Stock(newQty, newThreshold));
            }

            if (dto.sku.has_value()) {
                // Verificar se SKU já existe (em outro produto)
                if (!dto.sku->empty()) {
                    auto existingBySku = repository_->findBySku(dto.tenantId, *dto.sku);
                    if (existingBySku.has_value() && existingBySku->getId() != dto.productId) {
                        return {false, "SKU already exists for another product"};
                    }
                }
                product.setSku(*dto.sku);
            }

            if (dto.barcode.has_value()) {
                product.setBarcode(*dto.barcode);
            }

            if (dto.weight.has_value()) {
                std::string unit = dto.weightUnit.value_or("kg");
                product.setWeight(*dto.weight, unit);
            }

            // 4. Salvar alterações
            bool updated = repository_->update(product);
            if (!updated) {
                return {false, "Failed to update product"};
            }

            return {true, "Product updated successfully"};

        } catch (const std::exception& e) {
            return {false, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<ProductRepository> repository_;
};

} // namespace Product
} // namespace Domain

