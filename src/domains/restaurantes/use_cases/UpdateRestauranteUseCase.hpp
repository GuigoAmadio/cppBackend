#pragma once

#include "../entities/RestauranteProfile.hpp"
#include "../repositories/IRestauranteRepository.hpp"
#include <memory>
#include <stdexcept>

namespace Domains::Restaurantes::UseCases {

using Domains::Restaurantes::Entities::RestauranteProfile;
using Domains::Restaurantes::Repositories::IRestauranteRepository;

/**
 * @brief Use Case para atualizar perfil de restaurante
 */
class UpdateRestauranteUseCase {
public:
    explicit UpdateRestauranteUseCase(std::shared_ptr<IRestauranteRepository> repository)
        : repository_(repository) {}

    bool execute(const RestauranteProfile& restaurante) {
        // Verificar se restaurante existe
        auto existing = repository_->findById(restaurante.getId());
        if (!existing.has_value()) {
            throw std::runtime_error("Restaurante não encontrado");
        }

        // Atualizar
        return repository_->update(restaurante);
    }

private:
    std::shared_ptr<IRestauranteRepository> repository_;
};

} // namespace Domains::Restaurantes::UseCases

