#pragma once

#include "../entities/FreelancerProfile.hpp"
#include "../repositories/IFreelancerRepository.hpp"
#include <memory>
#include <vector>

namespace Domains::Freelances::UseCases {

using Domains::Freelances::Entities::FreelancerProfile;
using Domains::Freelances::Repositories::IFreelancerRepository;

/**
 * @brief Use Case para buscar freelancers próximos
 */
class SearchNearbyFreelancersUseCase {
public:
    explicit SearchNearbyFreelancersUseCase(std::shared_ptr<IFreelancerRepository> repository)
        : repository_(repository) {}

    /**
     * @brief Busca freelancers próximos de uma coordenada
     * @param latitude Latitude do ponto de busca
     * @param longitude Longitude do ponto de busca
     * @param radiusKm Raio de busca em quilômetros (padrão: 10km)
     */
    std::vector<FreelancerProfile> execute(
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
    std::shared_ptr<IFreelancerRepository> repository_;
};

} // namespace Domains::Freelances::UseCases

