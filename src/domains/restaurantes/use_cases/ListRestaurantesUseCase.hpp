#pragma once

#include "../entities/RestauranteProfile.hpp"
#include "../repositories/IRestauranteRepository.hpp"
#include <memory>
#include <vector>
#include <optional>

namespace Domains::Restaurantes::UseCases {

using Domains::Restaurantes::Entities::RestauranteProfile;
using Domains::Restaurantes::Repositories::IRestauranteRepository;

/**
 * @brief Use Case para listar restaurantes com filtros
 */
class ListRestaurantesUseCase {
public:
    explicit ListRestaurantesUseCase(std::shared_ptr<IRestauranteRepository> repository)
        : repository_(repository) {}

    std::vector<RestauranteProfile> executeAll() {
        return repository_->findAll();
    }

    std::vector<RestauranteProfile> executeAtivos() {
        return repository_->findAtivos();
    }

    std::vector<RestauranteProfile> executeByCategoria(const std::string& categoria) {
        return repository_->findByCategoria(categoria);
    }

    std::vector<RestauranteProfile> executeByCidade(const std::string& cidade) {
        return repository_->findByCidade(cidade);
    }

private:
    std::shared_ptr<IRestauranteRepository> repository_;
};

} // namespace Domains::Restaurantes::UseCases

