#include "CustomerRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>

namespace Domain {
namespace Customer {

Customer CustomerRepositoryImpl::mapToCustomer(
    const Core::Database::QueryResult& result,
    int row
) {
    LOG_DEBUG("[CustomerRepository] Mapping row " + std::to_string(row) + " to Customer");
    
    std::string id = result.getValue(row, 0);
    std::string tenantId = result.getValue(row, 1);
    std::string name = result.getValue(row, 3);
    std::string typeStr = result.getValue(row, 7);
    std::string createdBy = result.getValue(row, 12);
    
    Customer customer(
        id, tenantId, name,
        CustomerType(typeStr),
        createdBy
    );
    
    // workspace_id (col 2)
    std::string workspaceId = result.getValue(row, 2);
    if (!workspaceId.empty()) {
        customer.setWorkspaceId(workspaceId);
    }
    
    // email (col 4)
    std::string email = result.getValue(row, 4);
    if (!email.empty()) {
        customer.setEmail(email);
    }
    
    // phone (col 5)
    std::string phone = result.getValue(row, 5);
    if (!phone.empty()) {
        customer.setPhone(phone);
    }
    
    // document (col 6)
    std::string document = result.getValue(row, 6);
    if (!document.empty()) {
        customer.setDocument(document);
    }
    
    // status (col 8)
    std::string statusStr = result.getValue(row, 8);
    if (!statusStr.empty()) {
        customer.setStatus(CustomerStatus(statusStr));
    }
    
    // address (col 9)
    std::string address = result.getValue(row, 9);
    if (!address.empty()) {
        customer.setAddress(address);
    }
    
    // notes (col 10)
    std::string notes = result.getValue(row, 10);
    if (!notes.empty()) {
        customer.setNotes(notes);
    }
    
    // created_at (col 13), updated_at (col 14)
    std::string createdAt = result.getValue(row, 13);
    if (!createdAt.empty()) {
        customer.setCreatedAt(createdAt);
    }
    
    std::string updatedAt = result.getValue(row, 14);
    if (!updatedAt.empty()) {
        customer.setUpdatedAt(updatedAt);
    }
    
    return customer;
}

bool CustomerRepositoryImpl::save(const Customer& customer) {
    LOG_DEBUG("[CustomerRepository] save() called - customer_id: " + customer.getId());
    LOG_DEBUG("[CustomerRepository] tenant: " + customer.getTenantId());
    LOG_DEBUG("[CustomerRepository] name: " + customer.getName());
    LOG_DEBUG("[CustomerRepository] type: " + customer.getType().toString());
    
    auto conn = pool_->acquire();
    LOG_DEBUG("[CustomerRepository] Connection acquired");
    
    std::string query = R"(
        INSERT INTO customers (
            id, tenant_id, workspace_id, name, email, phone, document,
            type, status, address, notes, created_by, created_at, updated_at
        ) VALUES (
            $1, $2, NULLIF($3,''), $4, NULLIF($5,''), NULLIF($6,''), NULLIF($7,''),
            $8, $9, NULLIF($10,'')::jsonb, NULLIF($11,''), $12, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP
        )
    )";
    
    std::vector<std::string> params;
    params.push_back(customer.getId());
    params.push_back(customer.getTenantId());
    params.push_back(customer.getWorkspaceId().value_or(""));
    params.push_back(customer.getName());
    params.push_back(customer.getEmail().value_or(""));
    params.push_back(customer.getPhone().value_or(""));
    params.push_back(customer.getDocument().value_or(""));
    params.push_back(customer.getType().toString());
    params.push_back(customer.getStatus().toString());
    params.push_back(customer.getAddress().value_or(""));  // JSON
    params.push_back(customer.getNotes().value_or(""));
    params.push_back(customer.getCreatedBy());
    
    LOG_DEBUG("[CustomerRepository] Parameters prepared, total: " + std::to_string(params.size()));
    LOG_DEBUG("[CustomerRepository] Executing INSERT...");
    
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        LOG_ERROR("[CustomerRepository] INSERT failed!");
        LOG_ERROR("[CustomerRepository] Error: " + result.getError());
        return false;
    }
    
    LOG_DEBUG("[CustomerRepository] Customer saved successfully");
    return true;
}

bool CustomerRepositoryImpl::update(const Customer& customer) {
    LOG_DEBUG("[CustomerRepository] update() called - customer_id: " + customer.getId());
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE customers SET
            name = $2,
            email = NULLIF($3,''),
            phone = NULLIF($4,''),
            document = NULLIF($5,''),
            status = $6,
            address = NULLIF($7,'')::jsonb,
            notes = NULLIF($8,''),
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $1
    )";
    
    std::vector<std::string> params;
    params.push_back(customer.getId());
    params.push_back(customer.getName());
    params.push_back(customer.getEmail().value_or(""));
    params.push_back(customer.getPhone().value_or(""));
    params.push_back(customer.getDocument().value_or(""));
    params.push_back(customer.getStatus().toString());
    params.push_back(customer.getAddress().value_or(""));
    params.push_back(customer.getNotes().value_or(""));
    
    LOG_DEBUG("[CustomerRepository] Executing UPDATE...");
    auto result = conn->executeParams(query, params);
    
    if (!result.isSuccess()) {
        LOG_ERROR("[CustomerRepository] UPDATE failed: " + result.getError());
        return false;
    }
    
    LOG_DEBUG("[CustomerRepository] Customer updated successfully");
    return true;
}

bool CustomerRepositoryImpl::remove(const std::string& id) {
    LOG_DEBUG("[CustomerRepository] remove() called - customer_id: " + id);
    
    auto conn = pool_->acquire();
    
    // Soft delete: marcar como inactive
    std::string query = R"(
        UPDATE customers SET
            status = 'inactive',
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $1
    )";
    
    auto result = conn->executeParams(query, {id});
    
    if (!result.isSuccess()) {
        LOG_ERROR("[CustomerRepository] Soft delete failed: " + result.getError());
        return false;
    }
    
    LOG_DEBUG("[CustomerRepository] Customer soft deleted");
    return true;
}

std::optional<Customer> CustomerRepositoryImpl::findById(const std::string& id) {
    LOG_DEBUG("[CustomerRepository] findById() called - id: " + id);
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, name, email, phone, document,
               type, status, address, notes, tags, created_by, created_at, updated_at
        FROM customers
        WHERE id = $1
    )";
    
    auto result = conn->executeParams(query, {id});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        LOG_DEBUG("[CustomerRepository] Customer not found");
        return std::nullopt;
    }
    
    LOG_DEBUG("[CustomerRepository] Customer found");
    return mapToCustomer(result, 0);
}

std::optional<Customer> CustomerRepositoryImpl::findByEmail(
    const std::string& tenantId,
    const std::string& email
) {
    LOG_DEBUG("[CustomerRepository] findByEmail() - email: " + email);
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, name, email, phone, document,
               type, status, address, notes, tags, created_by, created_at, updated_at
        FROM customers
        WHERE tenant_id = $1 AND email = $2
    )";
    
    auto result = conn->executeParams(query, {tenantId, email});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        return std::nullopt;
    }
    
    return mapToCustomer(result, 0);
}

std::optional<Customer> CustomerRepositoryImpl::findByDocument(
    const std::string& tenantId,
    const std::string& document
) {
    LOG_DEBUG("[CustomerRepository] findByDocument() - document: " + document);
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, name, email, phone, document,
               type, status, address, notes, tags, created_by, created_at, updated_at
        FROM customers
        WHERE tenant_id = $1 AND document = $2
    )";
    
    auto result = conn->executeParams(query, {tenantId, document});
    
    if (!result.isSuccess() || result.rowCount() == 0) {
        return std::nullopt;
    }
    
    return mapToCustomer(result, 0);
}

std::vector<Customer> CustomerRepositoryImpl::findByTenant(
    const std::string& tenantId,
    int limit,
    int offset
) {
    LOG_DEBUG("[CustomerRepository] findByTenant() - tenant: " + tenantId);
    
    std::vector<Customer> customers;
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, name, email, phone, document,
               type, status, address, notes, tags, created_by, created_at, updated_at
        FROM customers
        WHERE tenant_id = $1
        ORDER BY created_at DESC
        LIMIT $2 OFFSET $3
    )";
    
    auto result = conn->executeParams(query, {
        tenantId, 
        std::to_string(limit), 
        std::to_string(offset)
    });
    
    if (!result.isSuccess()) {
        LOG_ERROR("[CustomerRepository] Query failed: " + result.getError());
        return customers;
    }
    
    LOG_DEBUG("[CustomerRepository] Found " + std::to_string(result.rowCount()) + " customers");
    
    for (int i = 0; i < result.rowCount(); i++) {
        customers.push_back(mapToCustomer(result, i));
    }
    
    return customers;
}

std::vector<Customer> CustomerRepositoryImpl::search(
    const std::string& tenantId,
    const std::string& searchQuery,
    int limit
) {
    LOG_DEBUG("[CustomerRepository] search() - query: " + searchQuery);
    
    std::vector<Customer> customers;
    auto conn = pool_->acquire();
    
    std::string query = R"(
        SELECT id, tenant_id, workspace_id, name, email, phone, document,
               type, status, address, notes, tags, created_by, created_at, updated_at
        FROM customers
        WHERE tenant_id = $1 
          AND (name ILIKE $2 OR email ILIKE $2 OR document ILIKE $2)
        ORDER BY name
        LIMIT $3
    )";
    
    std::string searchPattern = "%" + searchQuery + "%";
    
    auto result = conn->executeParams(query, {
        tenantId,
        searchPattern,
        std::to_string(limit)
    });
    
    if (!result.isSuccess()) {
        LOG_ERROR("[CustomerRepository] Search failed: " + result.getError());
        return customers;
    }
    
    LOG_DEBUG("[CustomerRepository] Found " + std::to_string(result.rowCount()) + " customers");
    
    for (int i = 0; i < result.rowCount(); i++) {
        customers.push_back(mapToCustomer(result, i));
    }
    
    return customers;
}

} // namespace Customer
} // namespace Domain

