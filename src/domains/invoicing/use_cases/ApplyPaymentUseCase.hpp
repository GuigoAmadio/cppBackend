#pragma once

#include <memory>
#include "../repositories/InvoiceRepository.hpp"
#include "../entities/Invoice.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Invoicing {

struct ApplyPaymentDTO {
    std::string invoiceId;
    std::string tenantId;
    double amount;
    std::string updatedBy;
};

class ApplyPaymentUseCase {
public:
    explicit ApplyPaymentUseCase(std::shared_ptr<InvoiceRepository> repository)
        : repository_(repository) {}

    Invoice execute(const ApplyPaymentDTO& dto) {
        Core::Utils::Logger::info("[ApplyPaymentUseCase] Applying payment to invoice: " + dto.invoiceId);

        if (dto.invoiceId.empty()) {
            throw std::invalid_argument("Invoice ID is required");
        }
        if (dto.tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }
        if (dto.amount <= 0) {
            throw std::invalid_argument("Payment amount must be greater than zero");
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

        // Aplicar pagamento (a entidade já atualiza status automaticamente)
        invoice.applyPayment(dto.amount);

        if (!dto.updatedBy.empty()) {
            invoice.setUpdatedBy(dto.updatedBy);
        }

        // Salvar
        if (!repository_->save(invoice)) {
            throw std::runtime_error("Failed to update invoice");
        }

        Core::Utils::Logger::info("[ApplyPaymentUseCase] Payment applied successfully. New status: " + invoice.getStatus().toString());
        return invoice;
    }

private:
    std::shared_ptr<InvoiceRepository> repository_;
};

} // namespace Domain::Invoicing

