#pragma once

#include "../entities/RestauranteProfile.hpp"
#include "../repositories/IRestauranteRepository.hpp"
#include <memory>
#include <optional>

namespace Domains::Restaurantes::UseCases {

using Domains::Restaurantes::Entities::RestauranteProfile;
using Domains::Restaurantes::Repositories::IRestauranteRepository;

/**
 * @brief Use Case para buscar restaurante
 */
class GetRestauranteUseCase {
public:
    explicit GetRestauranteUseCase(std::shared_ptr<IRestauranteRepository> repository)
        : repository_(repository) {}

    std::optional<RestauranteProfile> executeById(const std::string& id) {
        return repository_->findById(id);
    }

    std::optional<RestauranteProfile> executeByUserId(const std::string& userId) {
        return repository_->findByUserId(userId);
    }

    std::optional<RestauranteProfile> executeByCnpj(const std::string& cnpj) {
        return repository_->findByCnpj(cnpj);
    }

private:
    std::shared_ptr<IRestauranteRepository> repository_;
};

} // namespace Domains::Restaurantes::UseCases

