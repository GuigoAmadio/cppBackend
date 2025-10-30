#pragma once

#include <memory>
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../use_cases/CreateInvoiceUseCase.hpp"
#include "../use_cases/GetInvoiceUseCase.hpp"
#include "../use_cases/UpdateInvoiceStatusUseCase.hpp"
#include "../use_cases/ApplyPaymentUseCase.hpp"
#include "../repositories/InvoiceRepository.hpp"

namespace Domain::Invoicing {

class InvoiceController {
public:
    InvoiceController(
        std::shared_ptr<CreateInvoiceUseCase> createUseCase,
        std::shared_ptr<GetInvoiceUseCase> getUseCase,
        std::shared_ptr<UpdateInvoiceStatusUseCase> updateStatusUseCase,
        std::shared_ptr<ApplyPaymentUseCase> applyPaymentUseCase,
        std::shared_ptr<InvoiceRepository> repository
    );

    // REST endpoints
    Core::Http::Response createInvoice(const Core::Http::Request& req);
    Core::Http::Response getInvoice(const Core::Http::Request& req);
    Core::Http::Response listInvoices(const Core::Http::Request& req);
    Core::Http::Response listCustomerInvoices(const Core::Http::Request& req);
    Core::Http::Response updateInvoiceStatus(const Core::Http::Request& req);
    Core::Http::Response applyPayment(const Core::Http::Request& req);
    Core::Http::Response getInvoiceItems(const Core::Http::Request& req);

private:
    std::shared_ptr<CreateInvoiceUseCase> createUseCase_;
    std::shared_ptr<GetInvoiceUseCase> getUseCase_;
    std::shared_ptr<UpdateInvoiceStatusUseCase> updateStatusUseCase_;
    std::shared_ptr<ApplyPaymentUseCase> applyPaymentUseCase_;
    std::shared_ptr<InvoiceRepository> repository_;

    std::string invoiceToJson(const Invoice& invoice);
    std::string invoiceItemToJson(const InvoiceItem& item);
    std::string getQueryParam(const Core::Http::Request& req, const std::string& key);
};

} // namespace Domain::Invoicing

