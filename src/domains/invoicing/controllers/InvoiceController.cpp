#include "InvoiceController.hpp"
#include "../../../core/utils/Logger.hpp"
#include "../../../core/json/Json.hpp"
#include <sstream>

namespace Domain::Invoicing {

InvoiceController::InvoiceController(
    std::shared_ptr<CreateInvoiceUseCase> createUseCase,
    std::shared_ptr<GetInvoiceUseCase> getUseCase,
    std::shared_ptr<UpdateInvoiceStatusUseCase> updateStatusUseCase,
    std::shared_ptr<ApplyPaymentUseCase> applyPaymentUseCase,
    std::shared_ptr<InvoiceRepository> repository
) : createUseCase_(createUseCase),
    getUseCase_(getUseCase),
    updateStatusUseCase_(updateStatusUseCase),
    applyPaymentUseCase_(applyPaymentUseCase),
    repository_(repository) {
    Core::Utils::Logger::info("[InvoiceController] Initialized");
}

std::string InvoiceController::getQueryParam(const Core::Http::Request& req, const std::string& key) {
    return req.getQuery(key);
}

Core::Http::Response InvoiceController::createInvoice(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[InvoiceController::createInvoice] Creating invoice");

    auto json = Core::Json::makeObject();
    
    try {
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        CreateInvoiceDTO dto;
        dto.tenantId = req.getCustomData("user_tenant_id");
        dto.createdBy = req.getCustomData("user_id");
        dto.customerId = bodyObj["customer_id"]->asString();
        dto.orderId = bodyObj.count("order_id") ? bodyObj["order_id"]->asString() : "";
        dto.subscriptionId = bodyObj.count("subscription_id") ? bodyObj["subscription_id"]->asString() : "";
        dto.invoiceType = bodyObj.count("invoice_type") ? bodyObj["invoice_type"]->asString() : "standard";
        dto.dueDate = bodyObj.count("due_date") ? bodyObj["due_date"]->asString() : "";
        dto.description = bodyObj.count("description") ? bodyObj["description"]->asString() : "";
        dto.notes = bodyObj.count("notes") ? bodyObj["notes"]->asString() : "";
        dto.terms = bodyObj.count("terms") ? bodyObj["terms"]->asString() : "";

        // Parse items array
        if (bodyObj.count("items") && bodyObj["items"]->isArray()) {
            for (const auto& itemJson : bodyObj["items"]->asArray()) {
                if (!itemJson->isObject()) continue;
                
                auto& itemObj = itemJson->asObject();
                InvoiceItemDTO item;
                item.productId = itemObj.count("product_id") ? itemObj["product_id"]->asString() : "";
                item.description = itemObj["description"]->asString();
                item.quantity = itemObj["quantity"]->asNumber();
                item.unitPrice = itemObj["unit_price"]->asNumber();
                item.discount = itemObj.count("discount") ? itemObj["discount"]->asNumber() : 0.0;
                item.taxRate = itemObj.count("tax_rate") ? itemObj["tax_rate"]->asNumber() : 0.0;
                dto.items.push_back(item);
            }
        }

        auto invoice = createUseCase_->execute(dto);
        
        // Padronizado: retornar apenas id, invoice_number e message (como outros controllers)
        json->asObject()["id"] = Core::Json::makeString(invoice.getId());
        json->asObject()["invoice_number"] = Core::Json::makeString(invoice.getInvoiceNumber());
        json->asObject()["status"] = Core::Json::makeString(invoice.getStatus().toString());
        json->asObject()["total"] = Core::Json::makeNumber(invoice.getTotal());
        json->asObject()["message"] = Core::Json::makeString("Invoice created successfully");
        return Core::Http::Response(Core::Http::StatusCode::Created).json(*json);

    } catch (const std::invalid_argument& e) {
        Core::Utils::Logger::error("[InvoiceController::createInvoice] Validation error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[InvoiceController::createInvoice] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::InternalServerError).json(*json);
    }
}

Core::Http::Response InvoiceController::getInvoice(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[InvoiceController::getInvoice] Getting invoice");

    auto json = Core::Json::makeObject();
    
    try {
        GetInvoiceDTO dto;
        dto.invoiceId = req.getParam("id");
        dto.tenantId = req.getCustomData("user_tenant_id");

        auto invoice = getUseCase_->execute(dto);

        if (!invoice) {
            json->asObject()["error"] = Core::Json::makeString("Invoice not found");
            return Core::Http::Response(Core::Http::StatusCode::NotFound).json(*json);
        }

        json->asObject()["invoice"] = Core::Json::makeString(invoiceToJson(*invoice));
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[InvoiceController::getInvoice] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::InternalServerError).json(*json);
    }
}

Core::Http::Response InvoiceController::listInvoices(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[InvoiceController::listInvoices] Listing invoices");

    auto json = Core::Json::makeObject();
    
    try {
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string status = getQueryParam(req, "status");

        std::vector<Invoice> invoices;

        if (!status.empty()) {
            invoices = repository_->findByStatus(tenantId, status);
        } else {
            invoices = repository_->findByTenant(tenantId);
        }

        auto invoicesArray = Core::Json::makeArray();
        for (const auto& invoice : invoices) {
            invoicesArray->asArray().push_back(Core::Json::makeString(invoiceToJson(invoice)));
        }

        json->asObject()["invoices"] = invoicesArray;
        json->asObject()["count"] = Core::Json::makeNumber(invoices.size());
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[InvoiceController::listInvoices] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::InternalServerError).json(*json);
    }
}

Core::Http::Response InvoiceController::listCustomerInvoices(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[InvoiceController::listCustomerInvoices] Listing customer invoices");

    auto json = Core::Json::makeObject();
    
    try {
        std::string customerId = req.getParam("customerId");
        auto invoices = repository_->findByCustomer(customerId);

        auto invoicesArray = Core::Json::makeArray();
        for (const auto& invoice : invoices) {
            invoicesArray->asArray().push_back(Core::Json::makeString(invoiceToJson(invoice)));
        }

        json->asObject()["invoices"] = invoicesArray;
        json->asObject()["count"] = Core::Json::makeNumber(invoices.size());
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[InvoiceController::listCustomerInvoices] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::InternalServerError).json(*json);
    }
}

Core::Http::Response InvoiceController::updateInvoiceStatus(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[InvoiceController::updateInvoiceStatus] Updating invoice status");

    auto json = Core::Json::makeObject();
    
    try {
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        UpdateInvoiceStatusDTO dto;
        dto.invoiceId = req.getParam("id");
        dto.tenantId = req.getCustomData("user_tenant_id");
        dto.updatedBy = req.getCustomData("user_id");
        dto.status = bodyObj["status"]->asString();

        auto invoice = updateStatusUseCase_->execute(dto);
        json->asObject()["invoice"] = Core::Json::makeString(invoiceToJson(invoice));
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::invalid_argument& e) {
        Core::Utils::Logger::error("[InvoiceController::updateInvoiceStatus] Validation error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[InvoiceController::updateInvoiceStatus] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::InternalServerError).json(*json);
    }
}

Core::Http::Response InvoiceController::applyPayment(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[InvoiceController::applyPayment] Applying payment to invoice");

    auto json = Core::Json::makeObject();
    
    try {
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        ApplyPaymentDTO dto;
        dto.invoiceId = req.getParam("id");
        dto.tenantId = req.getCustomData("user_tenant_id");
        dto.updatedBy = req.getCustomData("user_id");
        dto.amount = bodyObj["amount"]->asNumber();

        auto invoice = applyPaymentUseCase_->execute(dto);
        json->asObject()["invoice"] = Core::Json::makeString(invoiceToJson(invoice));
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::invalid_argument& e) {
        Core::Utils::Logger::error("[InvoiceController::applyPayment] Validation error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[InvoiceController::applyPayment] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::InternalServerError).json(*json);
    }
}

Core::Http::Response InvoiceController::getInvoiceItems(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[InvoiceController::getInvoiceItems] Getting invoice items");

    auto json = Core::Json::makeObject();
    
    try {
        std::string invoiceId = req.getParam("id");
        auto items = repository_->findItemsByInvoice(invoiceId);

        auto itemsArray = Core::Json::makeArray();
        for (const auto& item : items) {
            itemsArray->asArray().push_back(Core::Json::makeString(invoiceItemToJson(item)));
        }

        json->asObject()["items"] = itemsArray;
        json->asObject()["count"] = Core::Json::makeNumber(items.size());
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[InvoiceController::getInvoiceItems] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::InternalServerError).json(*json);
    }
}

std::string InvoiceController::invoiceToJson(const Invoice& invoice) {
    std::ostringstream json;
    json << "{"
         << "\"id\":\"" << invoice.getId() << "\""
         << ",\"invoice_number\":\"" << invoice.getInvoiceNumber() << "\""
         << ",\"tenant_id\":\"" << invoice.getTenantId() << "\""
         << ",\"customer_id\":\"" << invoice.getCustomerId() << "\"";

    if (invoice.getOrderId()) {
        json << ",\"order_id\":\"" << *invoice.getOrderId() << "\"";
    }
    if (invoice.getSubscriptionId()) {
        json << ",\"subscription_id\":\"" << *invoice.getSubscriptionId() << "\"";
    }

    json << ",\"subtotal\":" << invoice.getSubtotal()
         << ",\"tax\":" << invoice.getTax()
         << ",\"discount\":" << invoice.getDiscount()
         << ",\"total\":" << invoice.getTotal()
         << ",\"amount_paid\":" << invoice.getAmountPaid()
         << ",\"amount_due\":" << invoice.getAmountDue()
         << ",\"currency\":\"" << invoice.getCurrency() << "\""
         << ",\"status\":\"" << invoice.getStatus().toString() << "\""
         << ",\"invoice_type\":\"" << invoice.getType().toString() << "\""
         << ",\"issue_date\":\"" << invoice.getIssueDate() << "\"";

    if (invoice.getDueDate()) {
        json << ",\"due_date\":\"" << *invoice.getDueDate() << "\"";
    }
    if (invoice.getPaidAt()) {
        json << ",\"paid_at\":\"" << *invoice.getPaidAt() << "\"";
    }
    if (invoice.getDescription()) {
        json << ",\"description\":\"" << *invoice.getDescription() << "\"";
    }
    if (invoice.getNotes()) {
        json << ",\"notes\":\"" << *invoice.getNotes() << "\"";
    }

    json << "}";
    return json.str();
}

std::string InvoiceController::invoiceItemToJson(const InvoiceItem& item) {
    std::ostringstream json;
    json << "{"
         << "\"id\":\"" << item.getId() << "\""
         << ",\"invoice_id\":\"" << item.getInvoiceId() << "\"";

    if (item.getProductId()) {
        json << ",\"product_id\":\"" << *item.getProductId() << "\"";
    }

    json << ",\"description\":\"" << item.getDescription() << "\""
         << ",\"quantity\":" << item.getQuantity()
         << ",\"unit_price\":" << item.getUnitPrice()
         << ",\"discount\":" << item.getDiscount()
         << ",\"tax_rate\":" << item.getTaxRate()
         << ",\"tax_amount\":" << item.getTaxAmount()
         << ",\"total\":" << item.getTotal()
         << "}";

    return json.str();
}

} // namespace Domain::Invoicing
