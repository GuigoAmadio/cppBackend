#pragma once

#include <memory>
#include <string>
#include <optional>
#include "../repositories/ProductRepository.hpp"

namespace Domain {
namespace Product {

struct GetProductDto {
    std::string productId;
    std::string tenantId; // Para validação de multi-tenancy
};

struct GetProductResult {
    bool success;
    std::optional<Product> product;
    std::string message;
};

class GetProductUseCase {
public:
    explicit GetProductUseCase(std::shared_ptr<ProductRepository> repository)
        : repository_(repository) {}

    GetProductResult execute(const GetProductDto& dto) {
        try {
            // 1. Buscar produto
            auto productOpt = repository_->findById(dto.productId);
            
            if (!productOpt.has_value()) {
                return {false, std::nullopt, "Product not found"};
            }

            // 2. Validar tenant ownership (multi-tenancy)
            if (!dto.tenantId.empty() && productOpt->getTenantId() != dto.tenantId) {
                return {false, std::nullopt, "Access denied: product belongs to another tenant"};
            }

            return {true, productOpt, "Product retrieved successfully"};

        } catch (const std::exception& e) {
            return {false, std::nullopt, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<ProductRepository> repository_;
};

} // namespace Product
} // namespace Domain

