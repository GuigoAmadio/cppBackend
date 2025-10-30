#include "PaymentController.hpp"
#include "../../../core/utils/Logger.hpp"
#include "../../../core/json/Json.hpp"
#include <sstream>

namespace Domain::Payment {

PaymentController::PaymentController(
    std::shared_ptr<CreatePaymentUseCase> createUseCase,
    std::shared_ptr<ProcessPaymentUseCase> processUseCase,
    std::shared_ptr<RefundPaymentUseCase> refundUseCase,
    std::shared_ptr<CancelPaymentUseCase> cancelUseCase,
    std::shared_ptr<GetPaymentUseCase> getUseCase,
    std::shared_ptr<ListPaymentsUseCase> listUseCase,
    std::shared_ptr<PaymentRepository> repository
) : createUseCase_(createUseCase),
    processUseCase_(processUseCase),
    refundUseCase_(refundUseCase),
    cancelUseCase_(cancelUseCase),
    getUseCase_(getUseCase),
    listUseCase_(listUseCase),
    repository_(repository)
{
    Core::Utils::Logger::info("[PaymentController] Initialized");
}

Core::Http::Response PaymentController::createPayment(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[PaymentController::createPayment] Creating payment");

    auto json = Core::Json::makeObject();
    
    try {
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        CreatePaymentDTO dto;
        dto.orderId = bodyObj["order_id"]->asString();
        dto.customerId = bodyObj["customer_id"]->asString();
        dto.tenantId = req.getCustomData("user_tenant_id");
        dto.amount = bodyObj["amount"]->asNumber();
        dto.currency = bodyObj.count("currency") ? bodyObj["currency"]->asString() : "BRL";
        dto.provider = bodyObj["provider"]->asString();
        dto.paymentMethod = bodyObj.count("payment_method") ? bodyObj["payment_method"]->asString() : "";
        dto.createdBy = req.getCustomData("user_id");

        auto payment = createUseCase_->execute(dto);

        json->asObject()["payment"] = Core::Json::makeString(paymentToJson(payment));
        return Core::Http::Response(Core::Http::StatusCode::Created).json(*json);

    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[PaymentController::createPayment] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
    }
}

Core::Http::Response PaymentController::getPayment(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[PaymentController::getPayment] Getting payment");

    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        auto payment = getUseCase_->execute(id);

        if (!payment) {
            json->asObject()["error"] = Core::Json::makeString("Payment not found");
            return Core::Http::Response(Core::Http::StatusCode::NotFound).json(*json);
        }

        json->asObject()["payment"] = Core::Json::makeString(paymentToJson(*payment));
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[PaymentController::getPayment] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString("Failed to get payment");
        return Core::Http::Response(Core::Http::StatusCode::InternalServerError).json(*json);
    }
}

Core::Http::Response PaymentController::listPayments(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[PaymentController::listPayments] Listing payments");

    auto json = Core::Json::makeObject();
    
    try {
        ListPaymentsDTO dto;

        std::string orderId = req.getQuery("order_id");
        if (!orderId.empty()) dto.orderId = orderId;

        std::string customerId = req.getQuery("customer_id");
        if (!customerId.empty()) dto.customerId = customerId;

        std::string tenantId = req.getCustomData("user_tenant_id");
        if (!tenantId.empty()) dto.tenantId = tenantId;

        auto payments = listUseCase_->execute(dto);

        std::ostringstream jsonStr;
        jsonStr << "[";
        for (size_t i = 0; i < payments.size(); ++i) {
            if (i > 0) jsonStr << ",";
            jsonStr << paymentToJson(payments[i]);
        }
        jsonStr << "]";

        json->asObject()["payments"] = Core::Json::makeString(jsonStr.str());
        json->asObject()["count"] = Core::Json::makeNumber(payments.size());
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[PaymentController::listPayments] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString("Failed to list payments");
        return Core::Http::Response(Core::Http::StatusCode::InternalServerError).json(*json);
    }
}

Core::Http::Response PaymentController::processPayment(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[PaymentController::processPayment] Processing payment");

    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        ProcessPaymentDTO dto;
        dto.paymentId = id;
        dto.providerPaymentId = bodyObj["provider_payment_id"]->asString();
        dto.providerResponse = bodyObj.count("provider_response") ? bodyObj["provider_response"]->asString() : "";
        dto.userId = req.getCustomData("user_id");

        auto payment = processUseCase_->execute(dto);

        json->asObject()["payment"] = Core::Json::makeString(paymentToJson(payment));
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[PaymentController::processPayment] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
    }
}

Core::Http::Response PaymentController::refundPayment(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[PaymentController::refundPayment] Refunding payment");

    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        RefundPaymentDTO dto;
        dto.paymentId = id;
        dto.amount = bodyObj["amount"]->asNumber();
        dto.reason = bodyObj.count("reason") ? bodyObj["reason"]->asString() : "";
        dto.userId = req.getCustomData("user_id");

        auto payment = refundUseCase_->execute(dto);

        json->asObject()["payment"] = Core::Json::makeString(paymentToJson(payment));
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[PaymentController::refundPayment] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
    }
}

Core::Http::Response PaymentController::cancelPayment(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[PaymentController::cancelPayment] Cancelling payment");

    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        CancelPaymentDTO dto;
        dto.paymentId = id;
        dto.reason = bodyObj.count("reason") ? bodyObj["reason"]->asString() : "";
        dto.userId = req.getCustomData("user_id");

        auto payment = cancelUseCase_->execute(dto);

        json->asObject()["payment"] = Core::Json::makeString(paymentToJson(payment));
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[PaymentController::cancelPayment] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Core::Http::Response(Core::Http::StatusCode::BadRequest).json(*json);
    }
}

Core::Http::Response PaymentController::listTransactions(const Core::Http::Request& req) {
    Core::Utils::Logger::info("[PaymentController::listTransactions] Listing transactions");

    auto json = Core::Json::makeObject();
    
    try {
        std::string paymentId = req.getParam("id");
        auto transactions = repository_->findTransactionsByPaymentId(paymentId);

        std::ostringstream jsonStr;
        jsonStr << "[";
        for (size_t i = 0; i < transactions.size(); ++i) {
            if (i > 0) jsonStr << ",";
            jsonStr << transactionToJson(transactions[i]);
        }
        jsonStr << "]";

        json->asObject()["transactions"] = Core::Json::makeString(jsonStr.str());
        json->asObject()["count"] = Core::Json::makeNumber(transactions.size());
        return Core::Http::Response(Core::Http::StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Core::Utils::Logger::error("[PaymentController::listTransactions] Error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString("Failed to list transactions");
        return Core::Http::Response(Core::Http::StatusCode::InternalServerError).json(*json);
    }
}

std::string PaymentController::paymentToJson(const Payment& payment) {
    std::ostringstream json;
    json << "{"
         << "\"id\":\"" << payment.getId() << "\"";
    
    if (payment.getOrderId()) {
        json << ",\"order_id\":\"" << *payment.getOrderId() << "\"";
    }
    if (payment.getSubscriptionId()) {
        json << ",\"subscription_id\":\"" << *payment.getSubscriptionId() << "\"";
    }
    
    json << ",\"customer_id\":\"" << payment.getCustomerId() << "\""
         << ",\"tenant_id\":\"" << payment.getTenantId() << "\""
         << ",\"amount\":" << payment.getAmount()
         << ",\"currency\":\"" << payment.getCurrency() << "\""
         << ",\"provider\":\"" << payment.getProvider().toString() << "\""
         << ",\"status\":\"" << payment.getStatus().toString() << "\"";

    if (payment.getProviderPaymentId()) {
        json << ",\"provider_payment_id\":\"" << *payment.getProviderPaymentId() << "\"";
    }
    if (payment.getPaymentMethod()) {
        json << ",\"payment_method\":\"" << *payment.getPaymentMethod() << "\"";
    }
    if (payment.getPaidAt()) {
        json << ",\"paid_at\":\"" << *payment.getPaidAt() << "\"";
    }
    if (payment.getCreatedAt()) {
        json << ",\"created_at\":\"" << *payment.getCreatedAt() << "\"";
    }

    json << "}";
    return json.str();
}

std::string PaymentController::transactionToJson(const Transaction& transaction) {
    std::ostringstream json;
    json << "{"
         << "\"id\":\"" << transaction.getId() << "\","
         << "\"payment_id\":\"" << transaction.getPaymentId() << "\","
         << "\"type\":\"" << transaction.getType().toString() << "\","
         << "\"status\":\"" << transaction.getStatus() << "\","
         << "\"amount\":" << transaction.getAmount() << ","
         << "\"currency\":\"" << transaction.getCurrency() << "\"";

    if (transaction.getReason()) {
        json << ",\"reason\":\"" << *transaction.getReason() << "\"";
    }
    if (transaction.getCreatedAt()) {
        json << ",\"created_at\":\"" << *transaction.getCreatedAt() << "\"";
    }

    json << "}";
    return json.str();
}

} // namespace Domain::Payment
