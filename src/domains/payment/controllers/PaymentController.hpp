#pragma once

#include <memory>
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../use_cases/CreatePaymentUseCase.hpp"
#include "../use_cases/ProcessPaymentUseCase.hpp"
#include "../use_cases/RefundPaymentUseCase.hpp"
#include "../use_cases/CancelPaymentUseCase.hpp"
#include "../use_cases/GetPaymentUseCase.hpp"
#include "../use_cases/ListPaymentsUseCase.hpp"

namespace Domain::Payment {

/**
 * @brief Controller para gerenciar payments
 * 
 * Rotas:
 * POST   /api/payments - Criar payment
 * GET    /api/payments/:id - Buscar payment
 * GET    /api/payments - Listar payments (filtros: order_id, customer_id, tenant_id)
 * POST   /api/payments/:id/process - Processar/aprovar payment
 * POST   /api/payments/:id/refund - Reembolsar payment
 * POST   /api/payments/:id/cancel - Cancelar payment
 * GET    /api/payments/:id/transactions - Listar transações do payment
 */
class PaymentController {
public:
    PaymentController(
        std::shared_ptr<CreatePaymentUseCase> createUseCase,
        std::shared_ptr<ProcessPaymentUseCase> processUseCase,
        std::shared_ptr<RefundPaymentUseCase> refundUseCase,
        std::shared_ptr<CancelPaymentUseCase> cancelUseCase,
        std::shared_ptr<GetPaymentUseCase> getUseCase,
        std::shared_ptr<ListPaymentsUseCase> listUseCase,
        std::shared_ptr<PaymentRepository> repository
    );

    Core::Http::Response createPayment(const Core::Http::Request& req);
    Core::Http::Response getPayment(const Core::Http::Request& req);
    Core::Http::Response listPayments(const Core::Http::Request& req);
    Core::Http::Response processPayment(const Core::Http::Request& req);
    Core::Http::Response refundPayment(const Core::Http::Request& req);
    Core::Http::Response cancelPayment(const Core::Http::Request& req);
    Core::Http::Response listTransactions(const Core::Http::Request& req);

private:
    std::shared_ptr<CreatePaymentUseCase> createUseCase_;
    std::shared_ptr<ProcessPaymentUseCase> processUseCase_;
    std::shared_ptr<RefundPaymentUseCase> refundUseCase_;
    std::shared_ptr<CancelPaymentUseCase> cancelUseCase_;
    std::shared_ptr<GetPaymentUseCase> getUseCase_;
    std::shared_ptr<ListPaymentsUseCase> listUseCase_;
    std::shared_ptr<PaymentRepository> repository_;

    std::string paymentToJson(const Payment& payment);
    std::string transactionToJson(const Transaction& transaction);
};

} // namespace Domain::Payment

