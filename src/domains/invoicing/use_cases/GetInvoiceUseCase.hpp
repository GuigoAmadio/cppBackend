#pragma once

#include <memory>
#include "../repositories/InvoiceRepository.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Invoicing {

struct GetInvoiceDTO {
    std::string invoiceId;
    std::string tenantId;
};

class GetInvoiceUseCase {
public:
    explicit GetInvoiceUseCase(std::shared_ptr<InvoiceRepository> repository)
        : repository_(repository) {}

    std::optional<Invoice> execute(const GetInvoiceDTO& dto) {
        Core::Utils::Logger::info("[GetInvoiceUseCase] Getting invoice: " + dto.invoiceId);

        if (dto.invoiceId.empty()) {
            throw std::invalid_argument("Invoice ID is required");
        }
        if (dto.tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }

        auto invoice = repository_->findById(dto.invoiceId);

        if (!invoice) {
            Core::Utils::Logger::info("[GetInvoiceUseCase] Invoice not found");
            return std::nullopt;
        }

        // Verificar se a invoice pertence ao tenant
        if (invoice->getTenantId() != dto.tenantId) {
            Core::Utils::Logger::error("[GetInvoiceUseCase] Access denied to invoice");
            return std::nullopt;
        }

        return invoice;
    }

private:
    std::shared_ptr<InvoiceRepository> repository_;
};

} // namespace Domain::Invoicing

