#pragma once

#include "../entities/RestauranteProfile.hpp"
#include <vector>
#include <optional>
#include <memory>

namespace Domains::Restaurantes::Repositories {

using Domains::Restaurantes::Entities::RestauranteProfile;

/**
 * @brief Interface para repositório de restaurantes
 */
class IRestauranteRepository {
public:
    virtual ~IRestauranteRepository() = default;

    /**
     * @brief Cria um novo perfil de restaurante
     */
    virtual std::string create(const RestauranteProfile& restaurante) = 0;

    /**
     * @brief Busca restaurante por ID
     */
    virtual std::optional<RestauranteProfile> findById(const std::string& id) = 0;

    /**
     * @brief Busca restaurante por user_id
     */
    virtual std::optional<RestauranteProfile> findByUserId(const std::string& userId) = 0;

    /**
     * @brief Busca restaurante por CNPJ
     */
    virtual std::optional<RestauranteProfile> findByCnpj(const std::string& cnpj) = 0;

    /**
     * @brief Lista todos os restaurantes
     */
    virtual std::vector<RestauranteProfile> findAll() = 0;

    /**
     * @brief Lista restaurantes por categoria
     */
    virtual std::vector<RestauranteProfile> findByCategoria(const std::string& categoria) = 0;

    /**
     * @brief Lista restaurantes por cidade
     */
    virtual std::vector<RestauranteProfile> findByCidade(const std::string& cidade) = 0;

    /**
     * @brief Lista restaurantes ativos
     */
    virtual std::vector<RestauranteProfile> findAtivos() = 0;

    /**
     * @brief Atualiza perfil de restaurante
     */
    virtual bool update(const RestauranteProfile& restaurante) = 0;

    /**
     * @brief Deleta restaurante por ID
     */
    virtual bool deleteById(const std::string& id) = 0;

    /**
     * @brief Busca restaurantes próximos a uma coordenada (em KM)
     */
    virtual std::vector<RestauranteProfile> findNearby(
        double latitude, 
        double longitude, 
        double radiusKm
    ) = 0;
};

} // namespace Domains::Restaurantes::Repositories

