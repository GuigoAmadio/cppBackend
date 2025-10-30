#pragma once

#include "../entities/RestauranteProfile.hpp"
#include "../repositories/IRestauranteRepository.hpp"
#include <memory>
#include <stdexcept>

namespace Domains::Restaurantes::UseCases {

using Domains::Restaurantes::Entities::RestauranteProfile;
using Domains::Restaurantes::Repositories::IRestauranteRepository;

/**
 * @brief Use Case para criar perfil de restaurante
 */
class CreateRestauranteUseCase {
public:
    explicit CreateRestauranteUseCase(std::shared_ptr<IRestauranteRepository> repository)
        : repository_(repository) {}

    /**
     * @brief Cria um novo perfil de restaurante
     * @throws std::invalid_argument se dados inválidos
     * @throws std::runtime_error se CNPJ ou user_id já existem
     */
    std::string execute(const RestauranteProfile& restaurante) {
        // Validação: verificar se user_id já tem restaurante
        auto existingByUser = repository_->findByUserId(restaurante.getUserId());
        if (existingByUser.has_value()) {
            throw std::runtime_error("User já possui um perfil de restaurante");
        }

        // Validação: verificar se CNPJ já existe
        auto existingByCnpj = repository_->findByCnpj(restaurante.getCnpj());
        if (existingByCnpj.has_value()) {
            throw std::runtime_error("CNPJ já cadastrado");
        }

        // Validação: coordenadas são obrigatórias
        auto coords = restaurante.getCoordenadas();
        if (coords.getLatitude() == 0.0 && coords.getLongitude() == 0.0) {
            throw std::invalid_argument("Coordenadas são obrigatórias");
        }

        // Criar restaurante
        return repository_->create(restaurante);
    }

private:
    std::shared_ptr<IRestauranteRepository> repository_;
};

} // namespace Domains::Restaurantes::UseCases

