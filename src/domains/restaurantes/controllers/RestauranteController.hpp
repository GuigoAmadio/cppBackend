#pragma once

#include "../use_cases/CreateRestauranteUseCase.hpp"
#include "../use_cases/GetRestauranteUseCase.hpp"
#include "../use_cases/UpdateRestauranteUseCase.hpp"
#include "../use_cases/ListRestaurantesUseCase.hpp"
#include "../use_cases/SearchNearbyRestaurantesUseCase.hpp"
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../../../core/json/JsonValue.hpp"
#include <memory>

namespace Domains::Restaurantes::Controllers {

using Domains::Restaurantes::UseCases::CreateRestauranteUseCase;
using Domains::Restaurantes::UseCases::GetRestauranteUseCase;
using Domains::Restaurantes::UseCases::UpdateRestauranteUseCase;
using Domains::Restaurantes::UseCases::ListRestaurantesUseCase;
using Domains::Restaurantes::UseCases::SearchNearbyRestaurantesUseCase;
using Domains::Restaurantes::Entities::RestauranteProfile;
using Core::Http::Request;
using Core::Http::Response;
using Core::Json::JsonValue;

/**
 * @brief Controller para endpoints de Restaurantes
 * 
 * Rotas:
 * - POST   /api/restaurantes              - Criar perfil de restaurante
 * - GET    /api/restaurantes              - Listar restaurantes (com filtros)
 * - GET    /api/restaurantes/:id          - Buscar por ID
 * - GET    /api/restaurantes/user/:userId - Buscar por user_id
 * - PUT    /api/restaurantes/:id          - Atualizar perfil
 * - DELETE /api/restaurantes/:id          - Deletar perfil
 * - GET    /api/restaurantes/nearby       - Buscar próximos (?lat=&lng=&radius=)
 * - GET    /api/restaurantes/map          - Para exibir no mapa
 */
class RestauranteController {
public:
    RestauranteController(
        std::shared_ptr<CreateRestauranteUseCase> createUseCase,
        std::shared_ptr<GetRestauranteUseCase> getUseCase,
        std::shared_ptr<UpdateRestauranteUseCase> updateUseCase,
        std::shared_ptr<ListRestaurantesUseCase> listUseCase,
        std::shared_ptr<SearchNearbyRestaurantesUseCase> searchNearbyUseCase
    );

    /**
     * @brief POST /api/restaurantes
     * Body: {
     *   "userId": "...",
     *   "cnpj": "...",
     *   "razaoSocial": "...",
     *   "nomeFantasia": "...",
     *   "descricao": "...",
     *   "categoria": "restaurante|bar|hotel|cafeteria",
     *   "latitude": -23.5505,
     *   "longitude": -46.6333,
     *   "enderecoCompleto": "...",
     *   "cep": "...",
     *   "cidade": "...",
     *   "estado": "..."
     * }
     */
    Response create(const Request& req);

    /**
     * @brief GET /api/restaurantes?categoria=&cidade=&ativo=
     */
    Response list(const Request& req);

    /**
     * @brief GET /api/restaurantes/:id
     */
    Response getById(const Request& req);

    /**
     * @brief GET /api/restaurantes/user/:userId
     */
    Response getByUserId(const Request& req);

    /**
     * @brief PUT /api/restaurantes/:id
     */
    Response update(const Request& req);

    /**
     * @brief DELETE /api/restaurantes/:id
     */
    Response deleteById(const Request& req);

    /**
     * @brief GET /api/restaurantes/nearby?lat=&lng=&radius=
     */
    Response nearby(const Request& req);

    /**
     * @brief GET /api/restaurantes/map
     * Retorna restaurantes para exibir no mapa (ativos, com coordenadas)
     */
    Response map(const Request& req);

private:
    std::shared_ptr<CreateRestauranteUseCase> createUseCase_;
    std::shared_ptr<GetRestauranteUseCase> getUseCase_;
    std::shared_ptr<UpdateRestauranteUseCase> updateUseCase_;
    std::shared_ptr<ListRestaurantesUseCase> listUseCase_;
    std::shared_ptr<SearchNearbyRestaurantesUseCase> searchNearbyUseCase_;

    // Helpers para serialização
    std::shared_ptr<JsonValue> restauranteToJson(const RestauranteProfile& restaurante);
    RestauranteProfile jsonToRestaurante(const std::shared_ptr<JsonValue>& json);
};

} // namespace Domains::Restaurantes::Controllers

