#pragma once

#include "../use_cases/CreateVagaUseCase.hpp"
#include "../use_cases/GetVagaUseCase.hpp"
#include "../use_cases/UpdateVagaUseCase.hpp"
#include "../use_cases/ListVagasUseCase.hpp"
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../../../core/json/JsonValue.hpp"
#include <memory>

namespace Domains::Vagas::Controllers {

using Domains::Vagas::UseCases::CreateVagaUseCase;
using Domains::Vagas::UseCases::GetVagaUseCase;
using Domains::Vagas::UseCases::UpdateVagaUseCase;
using Domains::Vagas::UseCases::ListVagasUseCase;
using Domains::Vagas::Entities::Vaga;
using Core::Http::Request;
using Core::Http::Response;
using Core::Json::JsonValue;

/**
 * @brief Controller para endpoints de Vagas
 * 
 * Rotas:
 * - POST   /api/vagas - Criar vaga
 * - GET    /api/vagas - Listar vagas (filtros: categoria, restauranteId, status)
 * - GET    /api/vagas/:id - Buscar por ID
 * - PUT    /api/vagas/:id - Atualizar vaga
 * - DELETE /api/vagas/:id - Deletar vaga
 * - POST   /api/vagas/:id/close - Fechar vaga manualmente
 */
class VagaController {
public:
    VagaController(
        std::shared_ptr<CreateVagaUseCase> createUseCase,
        std::shared_ptr<GetVagaUseCase> getUseCase,
        std::shared_ptr<UpdateVagaUseCase> updateUseCase,
        std::shared_ptr<ListVagasUseCase> listUseCase
    );

    Response create(const Request& req);
    Response list(const Request& req);
    Response getById(const Request& req);
    Response update(const Request& req);
    Response deleteById(const Request& req);
    Response close(const Request& req);

private:
    std::shared_ptr<CreateVagaUseCase> createUseCase_;
    std::shared_ptr<GetVagaUseCase> getUseCase_;
    std::shared_ptr<UpdateVagaUseCase> updateUseCase_;
    std::shared_ptr<ListVagasUseCase> listUseCase_;

    std::shared_ptr<JsonValue> vagaToJson(const Vaga& vaga);
    Vaga jsonToVaga(const std::shared_ptr<JsonValue>& json);
};

} // namespace Domains::Vagas::Controllers

