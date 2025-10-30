#pragma once

#include <memory>
#include <optional>
#include "../repositories/CustomerRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Customer {

struct UpdateCustomerDto {
    std::string customerId;
    std::string tenantId;  // Para validação
    
    // Campos atualizáveis (todos opcionais)
    std::optional<std::string> name;
    std::optional<std::string> email;
    std::optional<std::string> phone;
    std::optional<std::string> document;
    std::optional<std::string> status;  // active, inactive, blocked, prospect
    std::optional<std::string> address; // JSON
    std::optional<std::string> notes;
};

struct UpdateCustomerResult {
    bool success;
    std::string message;
};

class UpdateCustomerUseCase {
public:
    explicit UpdateCustomerUseCase(std::shared_ptr<CustomerRepository> repository)
        : repository_(repository) {}

    UpdateCustomerResult execute(const UpdateCustomerDto& dto) {
        try {
            LOG_DEBUG("[UpdateCustomerUseCase] Updating customer: " + dto.customerId);
            
            // 1. Buscar customer existente
            auto customerOpt = repository_->findById(dto.customerId);
            if (!customerOpt.has_value()) {
                LOG_WARNING("[UpdateCustomerUseCase] Customer not found");
                return {false, "Customer not found"};
            }
            
            Customer customer = *customerOpt;
            
            // 2. Validar tenant ownership
            if (!dto.tenantId.empty() && customer.getTenantId() != dto.tenantId) {
                LOG_WARNING("[UpdateCustomerUseCase] Access denied");
                return {false, "Access denied"};
            }
            
            // 3. Atualizar campos fornecidos
            if (dto.name.has_value()) {
                customer.setName(*dto.name);
            }
            if (dto.email.has_value()) {
                customer.setEmail(*dto.email);
            }
            if (dto.phone.has_value()) {
                customer.setPhone(*dto.phone);
            }
            if (dto.document.has_value()) {
                customer.setDocument(*dto.document);
            }
            if (dto.status.has_value()) {
                customer.setStatus(CustomerStatus(*dto.status));
            }
            if (dto.address.has_value()) {
                customer.setAddress(*dto.address);
            }
            if (dto.notes.has_value()) {
                customer.setNotes(*dto.notes);
            }
            
            // 4. Salvar alterações
            LOG_DEBUG("[UpdateCustomerUseCase] Calling repository update()");
            bool updated = repository_->update(customer);
            
            if (!updated) {
                LOG_ERROR("[UpdateCustomerUseCase] Failed to update");
                return {false, "Failed to update customer"};
            }
            
            LOG_DEBUG("[UpdateCustomerUseCase] Customer updated successfully");
            return {true, "Customer updated successfully"};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[UpdateCustomerUseCase] Exception: " + std::string(e.what()));
            return {false, std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<CustomerRepository> repository_;
};

} // namespace Customer
} // namespace Domain

