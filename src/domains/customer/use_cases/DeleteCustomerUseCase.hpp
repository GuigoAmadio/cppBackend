#pragma once

#include <memory>
#include "../repositories/CustomerRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Customer {

struct DeleteCustomerDto {
    std::string customerId;
    std::string tenantId;  // Para validação
};

struct DeleteCustomerResult {
    bool success;
    std::string message;
};

class DeleteCustomerUseCase {
public:
    explicit DeleteCustomerUseCase(std::shared_ptr<CustomerRepository> repository)
        : repository_(repository) {}

    DeleteCustomerResult execute(const DeleteCustomerDto& dto) {
        try {
            LOG_DEBUG("[DeleteCustomerUseCase] Deleting customer: " + dto.customerId);
            
            // 1. Validar que o customer existe e pertence ao tenant
            auto customerOpt = repository_->findById(dto.customerId);
            if (!customerOpt.has_value()) {
                LOG_WARNING("[DeleteCustomerUseCase] Customer not found");
                return {false, "Customer not found"};
            }
            
            if (!dto.tenantId.empty() && customerOpt->getTenantId() != dto.tenantId) {
                LOG_WARNING("[DeleteCustomerUseCase] Access denied");
                return {false, "Access denied"};
            }
            
            // 2. Soft delete
            LOG_DEBUG("[DeleteCustomerUseCase] Calling repository remove() (soft delete)");
            bool deleted = repository_->remove(dto.customerId);
            
            if (!deleted) {
                LOG_ERROR("[DeleteCustomerUseCase] Failed to delete");
                return {false, "Failed to delete customer"};
            }
            
            LOG_DEBUG("[DeleteCustomerUseCase] Customer deleted successfully");
            return {true, "Customer deleted successfully"};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[DeleteCustomerUseCase] Exception: " + std::string(e.what()));
            return {false, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<CustomerRepository> repository_;
};

} // namespace Customer
} // namespace Domain

