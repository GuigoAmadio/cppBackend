#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "../repositories/SubscriptionRepository.hpp"
#include "../entities/Subscription.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Subscription {

struct CancelSubscriptionDTO {
    std::string subscriptionId;
    std::string reason;
    bool immediate = false;  // true = cancela imediatamente, false = cancela no fim do período
    std::string cancelledBy;
};

class CancelSubscriptionUseCase {
public:
    explicit CancelSubscriptionUseCase(std::shared_ptr<SubscriptionRepository> repository)
        : repository_(repository) {}

    Subscription execute(const CancelSubscriptionDTO& dto) {
        Utils::Logger::info("[CancelSubscriptionUseCase] Cancelling subscription: " + dto.subscriptionId);

        // Validações
        if (dto.subscriptionId.empty()) {
            throw std::invalid_argument("Subscription ID is required");
        }

        // Buscar subscription
        auto subOpt = repository_->findById(dto.subscriptionId);
        if (!subOpt) {
            throw std::runtime_error("Subscription not found: " + dto.subscriptionId);
        }
        auto subscription = *subOpt;

        // Verificar se pode cancelar
        if (!subscription.canBeCancelled()) {
            throw std::runtime_error("Subscription cannot be cancelled in current state: " + subscription.getStatus().toString());
        }

        // Timestamp atual
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&nowTime), "%Y-%m-%d %H:%M:%S");
        std::string cancelledAt = ss.str();

        // Aplicar cancelamento
        subscription.cancel(dto.reason);
        subscription.setCancelledAt(cancelledAt);
        
        if (dto.immediate) {
            // Cancelamento imediato - expira agora
            subscription.setExpiresAt(cancelledAt);
        } else {
            // Cancelamento no fim do período - expira na próxima data de cobrança
            if (subscription.getCurrentPeriodEnd()) {
                subscription.setExpiresAt(*subscription.getCurrentPeriodEnd());
            } else {
                subscription.setExpiresAt(cancelledAt);
            }
        }

        if (!dto.cancelledBy.empty()) {
            subscription.setUpdatedBy(dto.cancelledBy);
        }

        // Salvar
        if (!repository_->save(subscription)) {
            Utils::Logger::error("[CancelSubscriptionUseCase] Failed to save subscription");
            throw std::runtime_error("Failed to cancel subscription");
        }

        Utils::Logger::info("[CancelSubscriptionUseCase] Subscription cancelled successfully!");
        return subscription;
    }

private:
    std::shared_ptr<SubscriptionRepository> repository_;
};

} // namespace Domain::Subscription

