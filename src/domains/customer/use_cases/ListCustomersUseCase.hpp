#pragma once

#include <memory>
#include <vector>
#include "../repositories/CustomerRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Customer {

struct ListCustomersDto {
    std::string tenantId;
    std::string searchQuery;  // Opcional: para busca
    int limit = 100;
    int offset = 0;
};

struct ListCustomersResult {
    bool success;
    std::vector<Customer> customers;
    int total;
    std::string message;
};

class ListCustomersUseCase {
public:
    explicit ListCustomersUseCase(std::shared_ptr<CustomerRepository> repository)
        : repository_(repository) {}

    ListCustomersResult execute(const ListCustomersDto& dto) {
        try {
            LOG_DEBUG("[ListCustomersUseCase] Starting execution");
            LOG_DEBUG("[ListCustomersUseCase] tenant: " + dto.tenantId);
            LOG_DEBUG("[ListCustomersUseCase] search: " + dto.searchQuery);
            
            if (dto.tenantId.empty()) {
                LOG_ERROR("[ListCustomersUseCase] Tenant ID is empty");
                return {false, {}, 0, "Tenant ID is required"};
            }
            
            std::vector<Customer> customers;
            
            if (!dto.searchQuery.empty()) {
                LOG_DEBUG("[ListCustomersUseCase] Using search");
                customers = repository_->search(dto.tenantId, dto.searchQuery, dto.limit);
            } else {
                LOG_DEBUG("[ListCustomersUseCase] Listing all");
                customers = repository_->findByTenant(dto.tenantId, dto.limit, dto.offset);
            }
            
            LOG_DEBUG("[ListCustomersUseCase] Found " + std::to_string(customers.size()) + " customers");
            return {true, customers, static_cast<int>(customers.size()), "Customers retrieved successfully"};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[ListCustomersUseCase] Exception: " + std::string(e.what()));
            return {false, {}, 0, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<CustomerRepository> repository_;
};

} // namespace Customer
} // namespace Domain

