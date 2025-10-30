#pragma once

#include <memory>
#include "../repositories/CustomerRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Customer {

struct GetCustomerDto {
    std::string customerId;
    std::string tenantId;  // Para validação
};

struct GetCustomerResult {
    bool success;
    std::optional<Customer> customer;
    std::string message;
};

class GetCustomerUseCase {
public:
    explicit GetCustomerUseCase(std::shared_ptr<CustomerRepository> repository)
        : repository_(repository) {}

    GetCustomerResult execute(const GetCustomerDto& dto) {
        try {
            LOG_DEBUG("[GetCustomerUseCase] Getting customer: " + dto.customerId);
            
            auto customerOpt = repository_->findById(dto.customerId);
            
            if (!customerOpt.has_value()) {
                LOG_DEBUG("[GetCustomerUseCase] Customer not found");
                return {false, std::nullopt, "Customer not found"};
            }
            
            // Validar tenant ownership
            if (!dto.tenantId.empty() && customerOpt->getTenantId() != dto.tenantId) {
                LOG_WARNING("[GetCustomerUseCase] Access denied - wrong tenant");
                return {false, std::nullopt, "Access denied"};
            }
            
            LOG_DEBUG("[GetCustomerUseCase] Customer found");
            return {true, customerOpt, "Customer retrieved successfully"};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[GetCustomerUseCase] Exception: " + std::string(e.what()));
            return {false, std::nullopt, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<CustomerRepository> repository_;
};

} // namespace Customer
} // namespace Domain

