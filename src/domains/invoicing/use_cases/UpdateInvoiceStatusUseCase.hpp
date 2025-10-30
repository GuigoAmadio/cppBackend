#pragma once

#include <memory>
#include "../repositories/InvoiceRepository.hpp"
#include "../value_objects/InvoiceStatus.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Invoicing {

struct UpdateInvoiceStatusDTO {
    std::string invoiceId;
    std::string tenantId;
    std::string status;
    std::string updatedBy;
};

class UpdateInvoiceStatusUseCase {
public:
    explicit UpdateInvoiceStatusUseCase(std::shared_ptr<InvoiceRepository> repository)
        : repository_(repository) {}

    Invoice execute(const UpdateInvoiceStatusDTO& dto) {
        Core::Utils::Logger::info("[UpdateInvoiceStatusUseCase] Updating invoice: " + dto.invoiceId);

        if (dto.invoiceId.empty()) {
            throw std::invalid_argument("Invoice ID is required");
        }
        if (dto.tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }
        if (dto.status.empty()) {
            throw std::invalid_argument("Status is required");
        }

        auto invoiceOpt = repository_->findById(dto.invoiceId);
        if (!invoiceOpt) {
            throw std::runtime_error("Invoice not found");
        }

        Invoice invoice = *invoiceOpt;

        // Verificar tenant
        if (invoice.getTenantId() != dto.tenantId) {
            throw std::runtime_error("Access denied to invoice");
        }

        // Atualizar status
        InvoiceStatus newStatus(dto.status);

        if (newStatus.isSent()) {
            invoice.markAsSent();
        } else if (newStatus.isPaid()) {
            invoice.markAsPaid();
        } else if (newStatus.isCancelled()) {
            invoice.markAsCancelled();
        } else if (newStatus.isVoid()) {
            invoice.markAsVoid();
        } else {
            invoice.setStatus(newStatus);
        }

        if (!dto.updatedBy.empty()) {
            invoice.setUpdatedBy(dto.updatedBy);
        }

        // Salvar
        if (!repository_->save(invoice)) {
            throw std::runtime_error("Failed to update invoice");
        }

        Core::Utils::Logger::info("[UpdateInvoiceStatusUseCase] Invoice status updated successfully");
        return invoice;
    }

private:
    std::shared_ptr<InvoiceRepository> repository_;
};

} // namespace Domain::Invoicing

