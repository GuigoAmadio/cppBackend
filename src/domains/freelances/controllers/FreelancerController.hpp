#pragma once

#include "../use_cases/CreateFreelancerUseCase.hpp"
#include "../use_cases/GetFreelancerUseCase.hpp"
#include "../use_cases/UpdateFreelancerUseCase.hpp"
#include "../use_cases/ListFreelancersUseCase.hpp"
#include "../use_cases/SearchNearbyFreelancersUseCase.hpp"
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../../../core/json/JsonValue.hpp"
#include <memory>

namespace Domains::Freelances::Controllers {

using Domains::Freelances::UseCases::CreateFreelancerUseCase;
using Domains::Freelances::UseCases::GetFreelancerUseCase;
using Domains::Freelances::UseCases::UpdateFreelancerUseCase;
using Domains::Freelances::UseCases::ListFreelancersUseCase;
using Domains::Freelances::UseCases::SearchNearbyFreelancersUseCase;
using Domains::Freelances::Entities::FreelancerProfile;
using Core::Http::Request;
using Core::Http::Response;
using Core::Json::JsonValue;

/**
 * @brief Controller para endpoints de Freelancers
 * 
 * Rotas:
 * - POST   /api/freelancers              - Criar perfil de freelancer
 * - GET    /api/freelancers              - Listar freelancers (com filtros)
 * - GET    /api/freelancers/:id          - Buscar por ID
 * - GET    /api/freelancers/user/:userId - Buscar por user_id
 * - PUT    /api/freelancers/:id          - Atualizar perfil
 * - DELETE /api/freelancers/:id          - Deletar perfil
 * - GET    /api/freelancers/nearby       - Buscar próximos (?lat=&lng=&radius=)
 */
class FreelancerController {
public:
    FreelancerController(
        std::shared_ptr<CreateFreelancerUseCase> createUseCase,
        std::shared_ptr<GetFreelancerUseCase> getUseCase,
        std::shared_ptr<UpdateFreelancerUseCase> updateUseCase,
        std::shared_ptr<ListFreelancersUseCase> listUseCase,
        std::shared_ptr<SearchNearbyFreelancersUseCase> searchNearbyUseCase
    );

    Response create(const Request& req);
    Response list(const Request& req);
    Response getById(const Request& req);
    Response getByUserId(const Request& req);
    Response update(const Request& req);
    Response deleteById(const Request& req);
    Response nearby(const Request& req);

private:
    std::shared_ptr<CreateFreelancerUseCase> createUseCase_;
    std::shared_ptr<GetFreelancerUseCase> getUseCase_;
    std::shared_ptr<UpdateFreelancerUseCase> updateUseCase_;
    std::shared_ptr<ListFreelancersUseCase> listUseCase_;
    std::shared_ptr<SearchNearbyFreelancersUseCase> searchNearbyUseCase_;

    std::shared_ptr<JsonValue> freelancerToJson(const FreelancerProfile& freelancer);
    FreelancerProfile jsonToFreelancer(const std::shared_ptr<JsonValue>& json);
};

} // namespace Domains::Freelances::Controllers

