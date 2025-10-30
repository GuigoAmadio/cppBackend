#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../repositories/PaymentRepository.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Payment {

struct ListPaymentsDTO {
    std::optional<std::string> orderId;
    std::optional<std::string> customerId;
    std::optional<std::string> tenantId;
};

class ListPaymentsUseCase {
public:
    explicit ListPaymentsUseCase(std::shared_ptr<PaymentRepository> repository)
        : repository_(repository) {}

    std::vector<Payment> execute(const ListPaymentsDTO& dto) {
        Utils::Logger::info("[ListPaymentsUseCase] Listing payments");

        std::vector<Payment> payments;

        if (dto.orderId) {
            Utils::Logger::info("[ListPaymentsUseCase] Filtering by order: " + *dto.orderId);
            payments = repository_->findByOrderId(*dto.orderId);
        } else if (dto.customerId) {
            Utils::Logger::info("[ListPaymentsUseCase] Filtering by customer: " + *dto.customerId);
            payments = repository_->findByCustomerId(*dto.customerId);
        } else if (dto.tenantId) {
            Utils::Logger::info("[ListPaymentsUseCase] Filtering by tenant: " + *dto.tenantId);
            payments = repository_->findByTenant(*dto.tenantId);
        } else {
            Utils::Logger::warning("[ListPaymentsUseCase] No filter provided");
        }

        Utils::Logger::info("[ListPaymentsUseCase] Found " + std::to_string(payments.size()) + " payment(s)");
        return payments;
    }

private:
    std::shared_ptr<PaymentRepository> repository_;
};

} // namespace Domain::Payment

