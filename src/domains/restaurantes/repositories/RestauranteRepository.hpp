#pragma once

#include "IRestauranteRepository.hpp"
#include "../../../core/database/ConnectionPool.hpp"
#include <memory>

namespace Domains::Restaurantes::Repositories {

using Core::Database::ConnectionPool;
using Core::Database::QueryResult;

/**
 * @brief Implementação do repositório de restaurantes
 */
class RestauranteRepository : public IRestauranteRepository {
public:
    explicit RestauranteRepository(std::shared_ptr<ConnectionPool> pool)
        : pool_(pool) {}

    std::string create(const RestauranteProfile& restaurante) override;
    std::optional<RestauranteProfile> findById(const std::string& id) override;
    std::optional<RestauranteProfile> findByUserId(const std::string& userId) override;
    std::optional<RestauranteProfile> findByCnpj(const std::string& cnpj) override;
    std::vector<RestauranteProfile> findAll() override;
    std::vector<RestauranteProfile> findByCategoria(const std::string& categoria) override;
    std::vector<RestauranteProfile> findByCidade(const std::string& cidade) override;
    std::vector<RestauranteProfile> findAtivos() override;
    bool update(const RestauranteProfile& restaurante) override;
    bool deleteById(const std::string& id) override;
    std::vector<RestauranteProfile> findNearby(double latitude, double longitude, double radiusKm) override;

private:
    std::shared_ptr<ConnectionPool> pool_;

    /**
     * @brief Mapeia resultado do banco para entity
     */
    RestauranteProfile mapToRestaurante(const QueryResult& result, int row);
};

} // namespace Domains::Restaurantes::Repositories

