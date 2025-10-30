#pragma once

#include <memory>
#include <string>
#include "../repositories/PaymentRepository.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Payment {

class GetPaymentUseCase {
public:
    explicit GetPaymentUseCase(std::shared_ptr<PaymentRepository> repository)
        : repository_(repository) {}

    std::optional<Payment> execute(const std::string& paymentId) {
        Utils::Logger::info("[GetPaymentUseCase] Getting payment: " + paymentId);

        auto payment = repository_->findById(paymentId);
        
        if (payment) {
            Utils::Logger::info("[GetPaymentUseCase] Payment found!");
        } else {
            Utils::Logger::warning("[GetPaymentUseCase] Payment not found");
        }

        return payment;
    }

private:
    std::shared_ptr<PaymentRepository> repository_;
};

} // namespace Domain::Payment

