#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <random>
#include <iomanip>
#include "../repositories/ProductRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Product {

// Helper inline para gerar UUID simples (não usa crypto, mas suficiente para IDs únicos)
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

struct CreateProductDto {
    std::string tenantId;
    std::string code;
    std::string name;
    std::string type;  // "physical", "digital", "service"
    double price;
    std::string currency = "BRL";
    std::string createdBy;
    
    // Campos opcionais
    std::string workspaceId;
    std::string categoryId;
    std::string description;
    double cost = 0.0;
    bool trackInventory = true;
    int stockQuantity = 0;
    int lowStockThreshold = 10;
    std::string sku;
    std::string barcode;
    double weight = 0.0;
    std::string weightUnit = "kg";
};

struct CreateProductResult {
    bool success;
    std::string productId;
    std::string message;
};

class CreateProductUseCase {
public:
    explicit CreateProductUseCase(std::shared_ptr<ProductRepository> repository)
        : repository_(repository) {}

    CreateProductResult execute(const CreateProductDto& dto) {
        try {
            LOG_DEBUG("[CreateProductUseCase] Starting execution");
            LOG_DEBUG("[CreateProductUseCase] tenant_id: " + dto.tenantId);
            LOG_DEBUG("[CreateProductUseCase] code: " + dto.code);
            LOG_DEBUG("[CreateProductUseCase] name: " + dto.name);
            LOG_DEBUG("[CreateProductUseCase] createdBy: " + dto.createdBy);
            
            // 1. Validar campos obrigatórios
            if (dto.tenantId.empty()) {
                LOG_ERROR("[CreateProductUseCase] Tenant ID is empty");
                return {false, "", "Tenant ID is required"};
            }
            if (dto.code.empty()) {
                LOG_ERROR("[CreateProductUseCase] Product code is empty");
                return {false, "", "Product code is required"};
            }
            if (dto.name.empty()) {
                LOG_ERROR("[CreateProductUseCase] Product name is empty");
                return {false, "", "Product name is required"};
            }
            if (dto.createdBy.empty()) {
                LOG_ERROR("[CreateProductUseCase] Created by is empty");
                return {false, "", "Created by user ID is required"};
            }
            
            LOG_DEBUG("[CreateProductUseCase] All required fields validated");

            // 2. Verificar se código já existe
            LOG_DEBUG("[CreateProductUseCase] Checking if code already exists");
            auto existing = repository_->findByCode(dto.tenantId, dto.code);
            if (existing.has_value()) {
                LOG_WARNING("[CreateProductUseCase] Product code already exists: " + dto.code);
                return {false, "", "Product code already exists: " + dto.code};
            }

            // 3. Verificar SKU duplicado (se fornecido)
            if (!dto.sku.empty()) {
                LOG_DEBUG("[CreateProductUseCase] Checking if SKU already exists: " + dto.sku);
                auto existingBySku = repository_->findBySku(dto.tenantId, dto.sku);
                if (existingBySku.has_value()) {
                    LOG_WARNING("[CreateProductUseCase] Product SKU already exists: " + dto.sku);
                    return {false, "", "Product SKU already exists: " + dto.sku};
                }
            }

            // 4. Gerar ID do produto
            std::string productId = generateSimpleUUID();
            LOG_DEBUG("[CreateProductUseCase] Generated product ID: " + productId);
            
            Product product(
                productId,
                dto.tenantId,
                ProductCode(dto.code),
                dto.name,
                ProductType(dto.type),
                Price::fromDecimal(dto.price, dto.currency),
                dto.createdBy
            );

            // 5. Setar campos opcionais
            if (!dto.workspaceId.empty()) {
                product.setWorkspaceId(dto.workspaceId);
            }
            if (!dto.categoryId.empty()) {
                product.setCategoryId(dto.categoryId);
            }
            if (!dto.description.empty()) {
                product.setDescription(dto.description);
            }
            if (dto.cost > 0) {
                product.setCost(Price::fromDecimal(dto.cost, dto.currency));
            }
            
            product.setTrackInventory(dto.trackInventory);
            product.setStock(Stock(dto.stockQuantity, dto.lowStockThreshold));
            
            if (!dto.sku.empty()) {
                product.setSku(dto.sku);
            }
            if (!dto.barcode.empty()) {
                product.setBarcode(dto.barcode);
            }
            if (dto.weight > 0) {
                product.setWeight(dto.weight, dto.weightUnit);
            }

            // 6. Salvar no banco
            LOG_DEBUG("[CreateProductUseCase] Calling repository save()");
            bool saved = repository_->save(product);
            if (!saved) {
                LOG_ERROR("[CreateProductUseCase] Repository save() returned false");
                return {false, "", "Failed to save product"};
            }

            LOG_DEBUG("[CreateProductUseCase] Product saved successfully - ID: " + productId);
            return {true, productId, "Product created successfully"};

        } catch (const std::exception& e) {
            return {false, "", std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<ProductRepository> repository_;
};

} // namespace Product
} // namespace Domain

