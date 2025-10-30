#pragma once

#include <memory>
#include <string>
#include "../repositories/ProductRepository.hpp"

namespace Domain {
namespace Product {

struct DeleteProductDto {
    std::string productId;
    std::string tenantId; // Para validação
    bool softDelete = true; // Se true, apenas marca como archived; se false, deleta fisicamente
};

struct DeleteProductResult {
    bool success;
    std::string message;
};

class DeleteProductUseCase {
public:
    explicit DeleteProductUseCase(std::shared_ptr<ProductRepository> repository)
        : repository_(repository) {}

    DeleteProductResult execute(const DeleteProductDto& dto) {
        try {
            // 1. Buscar produto
            auto productOpt = repository_->findById(dto.productId);
            
            if (!productOpt.has_value()) {
                return {false, "Product not found"};
            }

            Product product = *productOpt;

            // 2. Validar tenant ownership
            if (!dto.tenantId.empty() && product.getTenantId() != dto.tenantId) {
                return {false, "Access denied: product belongs to another tenant"};
            }

            // 3. Verificar se produto já está arquivado
            if (product.getStatus().isArchived() && dto.softDelete) {
                return {false, "Product is already archived"};
            }

            // 4. Soft delete ou hard delete
            if (dto.softDelete) {
                // Soft delete: marcar como archived
                product.archive();
                bool updated = repository_->update(product);
                if (!updated) {
                    return {false, "Failed to archive product"};
                }
                return {true, "Product archived successfully"};
            } else {
                // Hard delete: remover fisicamente
                bool removed = repository_->remove(dto.productId);
                if (!removed) {
                    return {false, "Failed to delete product"};
                }
                return {true, "Product deleted successfully"};
            }

        } catch (const std::exception& e) {
            return {false, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<ProductRepository> repository_;
};

} // namespace Product
} // namespace Domain

