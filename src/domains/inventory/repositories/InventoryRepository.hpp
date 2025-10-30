#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>
#include "../entities/InventoryTransaction.hpp"
#include "../../../core/database/ConnectionPool.hpp"

namespace Domain {
namespace Inventory {

/**
 * @brief Interface: InventoryRepository
 * Repositório para operações de inventário
 */
class InventoryRepository {
public:
    virtual ~InventoryRepository() = default;

    // CRUD básico
    virtual bool save(const InventoryTransaction& transaction) = 0;
    virtual std::optional<InventoryTransaction> findById(const std::string& id) = 0;
    
    // Consultas
    virtual std::vector<InventoryTransaction> findByProduct(
        const std::string& tenantId, 
        const std::string& productId,
        int limit = 100
    ) = 0;
    
    virtual std::vector<InventoryTransaction> findByProductAndLocation(
        const std::string& tenantId,
        const std::string& productId,
        const std::string& locationId,
        int limit = 100
    ) = 0;
    
    virtual std::vector<InventoryTransaction> findByTenant(
        const std::string& tenantId,
        int limit = 100
    ) = 0;
    
    // Estoque atual por produto
    virtual int getCurrentStock(
        const std::string& tenantId,
        const std::string& productId
    ) = 0;
    
    virtual int getCurrentStockByLocation(
        const std::string& tenantId,
        const std::string& productId,
        const std::string& locationId
    ) = 0;
};

/**
 * @brief Implementação PostgreSQL do InventoryRepository
 */
class InventoryRepositoryImpl : public InventoryRepository {
public:
    explicit InventoryRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
        : pool_(pool) {}

    bool save(const InventoryTransaction& transaction) override;
    std::optional<InventoryTransaction> findById(const std::string& id) override;
    std::vector<InventoryTransaction> findByProduct(
        const std::string& tenantId, 
        const std::string& productId,
        int limit
    ) override;
    std::vector<InventoryTransaction> findByProductAndLocation(
        const std::string& tenantId,
        const std::string& productId,
        const std::string& locationId,
        int limit
    ) override;
    std::vector<InventoryTransaction> findByTenant(
        const std::string& tenantId,
        int limit
    ) override;
    int getCurrentStock(
        const std::string& tenantId,
        const std::string& productId
    ) override;
    int getCurrentStockByLocation(
        const std::string& tenantId,
        const std::string& productId,
        const std::string& locationId
    ) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    
    // Helper para converter QueryResult em InventoryTransaction
    InventoryTransaction mapToTransaction(const Core::Database::QueryResult& result, int row);
};

// Factory
inline std::shared_ptr<InventoryRepository> createInventoryRepository(
    std::shared_ptr<Core::Database::ConnectionPool> pool
) {
    return std::make_shared<InventoryRepositoryImpl>(pool);
}

} // namespace Inventory
} // namespace Domain

