#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include "../repositories/SubscriptionRepository.hpp"
#include "../entities/Subscription.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Subscription {

struct GetSubscriptionDTO {
    std::string subscriptionId;
    std::string tenantId;  // Para validação de acesso
};

class GetSubscriptionUseCase {
public:
    explicit GetSubscriptionUseCase(std::shared_ptr<SubscriptionRepository> repository)
        : repository_(repository) {}

    Subscription execute(const GetSubscriptionDTO& dto) {
        if (dto.subscriptionId.empty()) {
            throw std::invalid_argument("Subscription ID is required");
        }

        auto subOpt = repository_->findById(dto.subscriptionId);
        if (!subOpt) {
            throw std::runtime_error("Subscription not found");
        }

        auto subscription = *subOpt;

        // Validar acesso (tenant)
        if (!dto.tenantId.empty() && subscription.getTenantId() != dto.tenantId) {
            throw std::runtime_error("Access denied");
        }

        return subscription;
    }

private:
    std::shared_ptr<SubscriptionRepository> repository_;
};

struct ListSubscriptionsDTO {
    std::string tenantId;
    std::string customerId;  // Opcional: filtrar por customer
    std::string status;       // Opcional: filtrar por status
};

class ListSubscriptionsUseCase {
public:
    explicit ListSubscriptionsUseCase(std::shared_ptr<SubscriptionRepository> repository)
        : repository_(repository) {}

    std::vector<Subscription> execute(const ListSubscriptionsDTO& dto) {
        if (dto.tenantId.empty()) {
            throw std::invalid_argument("Tenant ID is required");
        }

        // Filtrar por customer se fornecido
        if (!dto.customerId.empty()) {
            return repository_->findByCustomer(dto.customerId);
        }

        // Filtrar por status se fornecido
        if (!dto.status.empty()) {
            return repository_->findByStatus(dto.tenantId, dto.status);
        }

        // Retornar todas do tenant
        return repository_->findByTenant(dto.tenantId);
    }

private:
    std::shared_ptr<SubscriptionRepository> repository_;
};

} // namespace Domain::Subscription

