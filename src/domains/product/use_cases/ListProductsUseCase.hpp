#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../repositories/ProductRepository.hpp"

namespace Domain {
namespace Product {

struct ListProductsDto {
    std::string tenantId;
    std::string workspaceId;  // Opcional: filtrar por workspace
    std::string categoryId;   // Opcional: filtrar por categoria
    std::string status;       // Opcional: filtrar por status
    std::string searchQuery;  // Opcional: busca textual
    bool onlyLowStock = false; // Opcional: apenas produtos com estoque baixo
    int limit = 100;
    int offset = 0;
};

struct ListProductsResult {
    bool success;
    std::vector<Product> products;
    int total;
    std::string message;
};

class ListProductsUseCase {
public:
    explicit ListProductsUseCase(std::shared_ptr<ProductRepository> repository)
        : repository_(repository) {}

    ListProductsResult execute(const ListProductsDto& dto) {
        try {
            // Validar tenant
            if (dto.tenantId.empty()) {
                return {false, {}, 0, "Tenant ID is required"};
            }

            std::vector<Product> products;

            // Determinar qual query executar baseado nos filtros
            if (dto.onlyLowStock) {
                // Produtos com estoque baixo
                products = repository_->findLowStock(dto.tenantId, dto.limit);
            }
            else if (!dto.searchQuery.empty()) {
                // Busca textual
                products = repository_->search(dto.tenantId, dto.searchQuery, dto.limit);
            }
            else if (!dto.workspaceId.empty()) {
                // Filtrar por workspace
                products = repository_->findByWorkspace(dto.workspaceId, dto.limit, dto.offset);
            }
            else if (!dto.categoryId.empty()) {
                // Filtrar por categoria
                products = repository_->findByCategory(dto.categoryId, dto.limit, dto.offset);
            }
            else if (!dto.status.empty()) {
                // Filtrar por status
                products = repository_->findByStatus(dto.tenantId, dto.status, dto.limit, dto.offset);
            }
            else {
                // Listar todos do tenant
                products = repository_->findByTenant(dto.tenantId, dto.limit, dto.offset);
            }

            // Contar total (considerando filtros)
            int total;
            if (!dto.workspaceId.empty()) {
                total = repository_->countByWorkspace(dto.workspaceId);
            } else {
                total = repository_->countByTenant(dto.tenantId);
            }

            return {true, products, total, "Products retrieved successfully"};

        } catch (const std::exception& e) {
            return {false, {}, 0, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<ProductRepository> repository_;
};

} // namespace Product
} // namespace Domain

