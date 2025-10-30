#pragma once

#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include "../repositories/InvoiceRepository.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Invoicing {

struct InvoiceItemDTO {
    std::string productId;
    std::string description;
    double quantity;
    double unitPrice;
    double discount = 0.0;
    double taxRate = 0.0;
};

struct CreateInvoiceDTO {
    std::string tenantId;
    std::string customerId;
    std::string orderId;
    std::string subscriptionId;
    std::string invoiceType = "standard";
    std::string dueDate;
    std::string description;
    std::string notes;
    std::string terms;
    std::string createdBy;
    std::vector<InvoiceItemDTO> items;
};

class CreateInvoiceUseCase {
public:
    explicit CreateInvoiceUseCase(std::shared_ptr<InvoiceRepository> repository)
        : repository_(repository) {}

    Invoice execute(const CreateInvoiceDTO& dto) {
        Core::Utils::Logger::info("[CreateInvoiceUseCase] Creating invoice for customer: " + dto.customerId);

        // Validações
        if (dto.tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }
        if (dto.customerId.empty()) {
            throw std::invalid_argument("Customer ID is required");
        }
        if (dto.orderId.empty() && dto.subscriptionId.empty()) {
            throw std::invalid_argument("Either Order ID or Subscription ID is required");
        }
        if (dto.items.empty()) {
            throw std::invalid_argument("Invoice must have at least one item");
        }

        // Gerar ID e número da fatura
        std::string invoiceId = generateInvoiceId();
        std::string invoiceNumber = generateInvoiceNumber(dto.tenantId);

        // Criar entidade Invoice
        InvoiceStatus draftStatus(InvoiceStatus::Type::DRAFT);
        InvoiceType invoiceType(dto.invoiceType);
        Invoice invoice(
            invoiceId,
            invoiceNumber,
            dto.tenantId,
            dto.customerId,
            draftStatus,
            invoiceType
        );

        // Configurar relações
        if (!dto.orderId.empty()) {
            invoice.setOrderId(dto.orderId);
        }
        if (!dto.subscriptionId.empty()) {
            invoice.setSubscriptionId(dto.subscriptionId);
        }

        // Configurar datas e metadados
        if (!dto.dueDate.empty()) {
            invoice.setDueDate(dto.dueDate);
        }
        if (!dto.description.empty()) {
            invoice.setDescription(dto.description);
        }
        if (!dto.notes.empty()) {
            invoice.setNotes(dto.notes);
        }
        if (!dto.terms.empty()) {
            invoice.setTerms(dto.terms);
        }
        if (!dto.createdBy.empty()) {
            invoice.setCreatedBy(dto.createdBy);
        }

        // Calcular totais dos itens
        double subtotal = 0.0;
        double totalTax = 0.0;
        double totalDiscount = 0.0;

        std::vector<InvoiceItem> items;
        int sortOrder = 0;

        for (const auto& itemDto : dto.items) {
            std::string itemId = generateItemId();
            
            InvoiceItem item(
                itemId,
                invoiceId,
                dto.tenantId,
                itemDto.description,
                itemDto.quantity,
                itemDto.unitPrice
            );

            if (!itemDto.productId.empty()) {
                item.setProductId(itemDto.productId);
            }
            item.setDiscount(itemDto.discount);
            item.setTaxRate(itemDto.taxRate);
            item.setSortOrder(sortOrder++);

            subtotal += item.getSubtotal();
            totalTax += item.getTaxAmount();
            totalDiscount += itemDto.discount;

            items.push_back(item);
        }

        // Atualizar valores da fatura
        invoice.setSubtotal(subtotal);
        invoice.setTax(totalTax);
        invoice.setDiscount(totalDiscount);
        // Total e amount_due são calculados automaticamente na entidade

        // Salvar invoice
        if (!repository_->save(invoice)) {
            throw std::runtime_error("Failed to save invoice");
        }

        // Salvar itens
        for (const auto& item : items) {
            if (!repository_->saveItem(item)) {
                Core::Utils::Logger::error("[CreateInvoiceUseCase] Failed to save item: " + item.getId());
                throw std::runtime_error("Failed to save invoice item");
            }
        }

        Core::Utils::Logger::info("[CreateInvoiceUseCase] Invoice created successfully: " + invoiceId);
        return invoice;
    }

private:
    std::shared_ptr<InvoiceRepository> repository_;

    std::string generateInvoiceId() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        return "inv-" + std::to_string(millis);
    }

    std::string generateItemId() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        return "item-" + std::to_string(nanos);
    }

    std::string generateInvoiceNumber(const std::string& tenantId) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << "INV-" << std::put_time(std::localtime(&time_t), "%Y%m%d-%H%M%S");
        return ss.str();
    }
};

} // namespace Domain::Invoicing

