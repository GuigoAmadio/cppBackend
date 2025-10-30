#pragma once

#include <memory>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include "../repositories/CustomerRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domain {
namespace Customer {

// Helper inline para gerar UUID
inline std::string generateCustomerUUID() {
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

struct CreateCustomerDto {
    std::string tenantId;
    std::string workspaceId;
    std::string name;
    std::string email;
    std::string phone;
    std::string document;  // CPF/CNPJ
    std::string type;      // individual, business, foreign
    std::string address;   // JSON
    std::string notes;
    std::string createdBy;
};

struct CreateCustomerResult {
    bool success;
    std::string customerId;
    std::string message;
};

class CreateCustomerUseCase {
public:
    explicit CreateCustomerUseCase(std::shared_ptr<CustomerRepository> repository)
        : repository_(repository) {}

    CreateCustomerResult execute(const CreateCustomerDto& dto) {
        try {
            LOG_DEBUG("[CreateCustomerUseCase] Starting execution");
            LOG_DEBUG("[CreateCustomerUseCase] tenant: " + dto.tenantId);
            LOG_DEBUG("[CreateCustomerUseCase] name: " + dto.name);
            LOG_DEBUG("[CreateCustomerUseCase] type: " + dto.type);
            
            // 1. Validar campos obrigatórios
            if (dto.tenantId.empty()) {
                LOG_ERROR("[CreateCustomerUseCase] Tenant ID is empty");
                return {false, "", "Tenant ID is required"};
            }
            if (dto.name.empty()) {
                LOG_ERROR("[CreateCustomerUseCase] Name is empty");
                return {false, "", "Customer name is required"};
            }
            if (dto.type.empty()) {
                LOG_ERROR("[CreateCustomerUseCase] Type is empty");
                return {false, "", "Customer type is required"};
            }
            if (dto.createdBy.empty()) {
                LOG_ERROR("[CreateCustomerUseCase] CreatedBy is empty");
                return {false, "", "Created by user ID is required"};
            }
            
            LOG_DEBUG("[CreateCustomerUseCase] All required fields validated");
            
            // 2. Verificar email duplicado (se fornecido)
            if (!dto.email.empty()) {
                LOG_DEBUG("[CreateCustomerUseCase] Checking if email already exists");
                auto existingByEmail = repository_->findByEmail(dto.tenantId, dto.email);
                if (existingByEmail.has_value()) {
                    LOG_WARNING("[CreateCustomerUseCase] Email already exists: " + dto.email);
                    return {false, "", "Email already registered for another customer"};
                }
            }
            
            // 3. Verificar documento duplicado (se fornecido)
            if (!dto.document.empty()) {
                LOG_DEBUG("[CreateCustomerUseCase] Checking if document already exists");
                auto existingByDoc = repository_->findByDocument(dto.tenantId, dto.document);
                if (existingByDoc.has_value()) {
                    LOG_WARNING("[CreateCustomerUseCase] Document already exists: " + dto.document);
                    return {false, "", "Document already registered for another customer"};
                }
            }
            
            // 4. Gerar ID
            std::string customerId = generateCustomerUUID();
            LOG_DEBUG("[CreateCustomerUseCase] Generated customer ID: " + customerId);
            
            // 5. Criar Customer
            Customer customer(
                customerId,
                dto.tenantId,
                dto.name,
                CustomerType(dto.type),
                dto.createdBy
            );
            
            // Campos opcionais
            if (!dto.workspaceId.empty()) {
                customer.setWorkspaceId(dto.workspaceId);
            }
            if (!dto.email.empty()) {
                customer.setEmail(dto.email);
            }
            if (!dto.phone.empty()) {
                customer.setPhone(dto.phone);
            }
            if (!dto.document.empty()) {
                customer.setDocument(dto.document);
            }
            if (!dto.address.empty()) {
                customer.setAddress(dto.address);
            }
            if (!dto.notes.empty()) {
                customer.setNotes(dto.notes);
            }
            
            // 6. Salvar
            LOG_DEBUG("[CreateCustomerUseCase] Calling repository save()");
            bool saved = repository_->save(customer);
            
            if (!saved) {
                LOG_ERROR("[CreateCustomerUseCase] Failed to save customer");
                return {false, "", "Failed to save customer"};
            }
            
            LOG_DEBUG("[CreateCustomerUseCase] Customer saved successfully");
            return {true, customerId, "Customer created successfully"};
            
        } catch (const std::exception& e) {
            LOG_ERROR("[CreateCustomerUseCase] Exception: " + std::string(e.what()));
            return {false, "", std::string("Error: ") + e.what()};
        }
    }

private:
    std::shared_ptr<CustomerRepository> repository_;
};

} // namespace Customer
} // namespace Domain

