#pragma once

#include "../entities/RestauranteProfile.hpp"
#include "../repositories/IRestauranteRepository.hpp"
#include <memory>
#include <vector>

namespace Domains::Restaurantes::UseCases {

using Domains::Restaurantes::Entities::RestauranteProfile;
using Domains::Restaurantes::Repositories::IRestauranteRepository;

/**
 * @brief Use Case para buscar restaurantes próximos
 */
class SearchNearbyRestaurantesUseCase {
public:
    explicit SearchNearbyRestaurantesUseCase(std::shared_ptr<IRestauranteRepository> repository)
        : repository_(repository) {}

    /**
     * @brief Busca restaurantes próximos de uma coordenada
     * @param latitude Latitude do ponto de busca
     * @param longitude Longitude do ponto de busca
     * @param radiusKm Raio de busca em quilômetros (padrão: 10km)
     */
    std::vector<RestauranteProfile> execute(
        double latitude,
        double longitude,
        double radiusKm = 10.0
    ) {
        if (radiusKm <= 0) {
            throw std::invalid_argument("Raio deve ser maior que zero");
        }

        return repository_->findNearby(latitude, longitude, radiusKm);
    }

private:
    std::shared_ptr<IRestauranteRepository> repository_;
};

} // namespace Domains::Restaurantes::UseCases

