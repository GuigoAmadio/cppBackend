#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>
#include "../entities/Customer.hpp"
#include "../../../core/database/ConnectionPool.hpp"

namespace Domain {
namespace Customer {

/**
 * @brief Interface: CustomerRepository
 */
class CustomerRepository {
public:
    virtual ~CustomerRepository() = default;

    virtual bool save(const Customer& customer) = 0;
    virtual bool update(const Customer& customer) = 0;
    virtual bool remove(const std::string& id) = 0;  // Soft delete
    
    virtual std::optional<Customer> findById(const std::string& id) = 0;
    virtual std::optional<Customer> findByEmail(const std::string& tenantId, const std::string& email) = 0;
    virtual std::optional<Customer> findByDocument(const std::string& tenantId, const std::string& document) = 0;
    
    virtual std::vector<Customer> findByTenant(
        const std::string& tenantId,
        int limit = 100,
        int offset = 0
    ) = 0;
    
    virtual std::vector<Customer> search(
        const std::string& tenantId,
        const std::string& searchQuery,
        int limit = 50
    ) = 0;
};

/**
 * @brief Implementação PostgreSQL
 */
class CustomerRepositoryImpl : public CustomerRepository {
public:
    explicit CustomerRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
        : pool_(pool) {}

    bool save(const Customer& customer) override;
    bool update(const Customer& customer) override;
    bool remove(const std::string& id) override;
    std::optional<Customer> findById(const std::string& id) override;
    std::optional<Customer> findByEmail(const std::string& tenantId, const std::string& email) override;
    std::optional<Customer> findByDocument(const std::string& tenantId, const std::string& document) override;
    std::vector<Customer> findByTenant(const std::string& tenantId, int limit, int offset) override;
    std::vector<Customer> search(const std::string& tenantId, const std::string& searchQuery, int limit) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    Customer mapToCustomer(const Core::Database::QueryResult& result, int row);
};

// Factory
inline std::shared_ptr<CustomerRepository> createCustomerRepository(
    std::shared_ptr<Core::Database::ConnectionPool> pool
) {
    return std::make_shared<CustomerRepositoryImpl>(pool);
}

} // namespace Customer
} // namespace Domain

