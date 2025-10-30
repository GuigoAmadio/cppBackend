#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include "../entities/Product.hpp"
#include "../../../core/database/ConnectionPool.hpp"

namespace Domain {
namespace Product {

/**
 * @brief Interface: ProductRepository
 * Define operações de persistência para produtos
 */
class ProductRepository {
public:
    virtual ~ProductRepository() = default;

    // CRUD básico
    virtual bool save(const Product& product) = 0;
    virtual bool update(const Product& product) = 0;
    virtual bool remove(const std::string& id) = 0;

    // Queries
    virtual std::optional<Product> findById(const std::string& id) = 0;
    virtual std::optional<Product> findByCode(const std::string& tenantId, const std::string& code) = 0;
    virtual std::optional<Product> findBySku(const std::string& tenantId, const std::string& sku) = 0;
    
    virtual std::vector<Product> findByTenant(const std::string& tenantId, int limit = 100, int offset = 0) = 0;
    virtual std::vector<Product> findByWorkspace(const std::string& workspaceId, int limit = 100, int offset = 0) = 0;
    virtual std::vector<Product> findByCategory(const std::string& categoryId, int limit = 100, int offset = 0) = 0;
    virtual std::vector<Product> findByStatus(const std::string& tenantId, const std::string& status, int limit = 100, int offset = 0) = 0;
    
    virtual std::vector<Product> search(const std::string& tenantId, const std::string& query, int limit = 100) = 0;
    virtual std::vector<Product> findLowStock(const std::string& tenantId, int limit = 100) = 0;

    // Contadores
    virtual int countByTenant(const std::string& tenantId) = 0;
    virtual int countByWorkspace(const std::string& workspaceId) = 0;
};

/**
 * @brief Implementação: ProductRepositoryImpl
 * Implementação PostgreSQL do ProductRepository
 */
class ProductRepositoryImpl : public ProductRepository {
public:
    explicit ProductRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
        : pool_(pool) {}

    bool save(const Product& product) override;
    bool update(const Product& product) override;
    bool remove(const std::string& id) override;

    std::optional<Product> findById(const std::string& id) override;
    std::optional<Product> findByCode(const std::string& tenantId, const std::string& code) override;
    std::optional<Product> findBySku(const std::string& tenantId, const std::string& sku) override;
    
    std::vector<Product> findByTenant(const std::string& tenantId, int limit, int offset) override;
    std::vector<Product> findByWorkspace(const std::string& workspaceId, int limit, int offset) override;
    std::vector<Product> findByCategory(const std::string& categoryId, int limit, int offset) override;
    std::vector<Product> findByStatus(const std::string& tenantId, const std::string& status, int limit, int offset) override;
    
    std::vector<Product> search(const std::string& tenantId, const std::string& query, int limit) override;
    std::vector<Product> findLowStock(const std::string& tenantId, int limit) override;

    int countByTenant(const std::string& tenantId) override;
    int countByWorkspace(const std::string& workspaceId) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    
    // Helper para converter QueryResult em Product
    Product resultToProduct(const Core::Database::QueryResult& result, int row);
};

// Factory function
std::shared_ptr<ProductRepository> createProductRepository(
    std::shared_ptr<Core::Database::ConnectionPool> pool
);

} // namespace Product
} // namespace Domain

