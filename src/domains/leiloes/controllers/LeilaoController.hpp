#pragma once

#include "../../../../src/core/http/Request.hpp"
#include "../../../../src/core/http/Response.hpp"
#include "../use_cases/CreateLeilaoUseCase.hpp"
#include "../repositories/LeilaoRepository.hpp"
#include "../../../../src/core/database/ConnectionPool.hpp"
#include "../../../../src/core/database/GlobalPool.hpp"
#include "../../../../src/core/utils/LoggerNew.hpp"

namespace Domains::Leiloes::Controllers {

using Core::Http::Request;
using Core::Http::Response;
using Core::Http::StatusCode;

class LeilaoController {
public:
    static Response create(const Request& req) {
        LOG_DEBUG(std::string("[Leiloes::create] body=") + req.getBody());
        auto body = req.getJson();
        if (!body || !body->isObject()) {
            auto error = Core::Json::makeObject();
            error->asObject()["error"] = Core::Json::makeString("Invalid JSON");
            LOG_WARNING("[Leiloes::create] invalid JSON");
            return Response(StatusCode::BadRequest).json(*error);
        }
        // Validar restaurante_id (sem fallback)
        std::string restauranteIdBody = body->get("restaurante_id") ? body->get("restaurante_id")->asString() : std::string("");
        auto conn = globalPool->acquire();
        std::string restauranteIdFinal = restauranteIdBody;
        try {
            auto r = conn->executeParams("SELECT 1 FROM restaurante_profiles WHERE id = $1 LIMIT 1", { restauranteIdFinal });
            if (!(r.isSuccess() && r.rowCount() > 0)) {
                auto error = Core::Json::makeObject();
                error->asObject()["status"] = Core::Json::makeString("error");
                error->asObject()["message"] = Core::Json::makeString("restaurante_id inválido ou inexistente");
                LOG_WARNING("[Leiloes::create] restaurante_id not found: " + restauranteIdFinal);
                return Response(StatusCode::BadRequest).json(*error);
            }
        } catch (...) {
            // fallback silencioso; se continuar vazio, o use case/DB acusará
        }

        Domains::Leiloes::UseCases::CreateLeilaoInput input{
            /*restauranteId*/ restauranteIdFinal,
            /*titulo*/ body->get("titulo")->asString(),
            /*categoria*/ body->get("categoria")->asString(),
            /*valorIdeal*/ body->get("valor_ideal")->asNumber(),
            /*valorMaxQuero*/ body->get("valor_max_quero")->asNumber(),
            /*dataTrabalho*/ body->get("data_trabalho")->asString(),
            /*duracaoHoras*/ std::nullopt,
            /*dataLimiteOffers*/ body->get("data_limite_offers")->asString()
        };
        Domains::Leiloes::Repositories::LeilaoRepository repo(globalPool);
        Domains::Leiloes::UseCases::CreateLeilaoUseCase uc(&repo);
        LOG_DEBUG("[Leiloes::create] executing use case");
        auto leilao = uc.execute(input);
        auto res = Core::Json::makeObject();
        res->asObject()["id"] = Core::Json::makeString(leilao.getId());
        res->asObject()["titulo"] = Core::Json::makeString(leilao.getTitulo());
        LOG_INFO("[Leiloes::create] created id=" + leilao.getId());
        return Response(StatusCode::OK).json(*res);
    }

    static Response list(const Request& req) {
        (void)req;
        Domains::Leiloes::Repositories::LeilaoRepository repo(globalPool);
        auto now = "2025-01-01T00:00:00Z"; // TODO: clock util
        LOG_DEBUG(std::string("[Leiloes::list] now=") + now);
        auto list = repo.listOpen(now);
        auto arr = Core::Json::makeArray();
        for (const auto& l : list) {
            auto obj = Core::Json::makeObject();
            obj->asObject()["id"] = Core::Json::makeString(l.getId());
            obj->asObject()["titulo"] = Core::Json::makeString(l.getTitulo());
            arr->asArray().push_back(obj);
        }
        LOG_INFO("[Leiloes::list] count=" + std::to_string(list.size()));
        return Response(StatusCode::OK).json(*arr);
    }

    static Response getById(const Request& req) {
        std::string id = req.getParam("id");
        Domains::Leiloes::Repositories::LeilaoRepository repo(globalPool);
        LOG_DEBUG("[Leiloes::getById] id=" + id);
        auto found = repo.findById(id);
        if (!found.has_value()) {
            auto err = Core::Json::makeObject();
            err->asObject()["error"] = Core::Json::makeString("not_found");
            LOG_WARNING("[Leiloes::getById] not found: " + id);
            return Response(StatusCode::NotFound).json(*err);
        }
        auto obj = Core::Json::makeObject();
        obj->asObject()["id"] = Core::Json::makeString(found->getId());
        obj->asObject()["titulo"] = Core::Json::makeString(found->getTitulo());
        LOG_INFO("[Leiloes::getById] ok id=" + id);
        return Response(StatusCode::OK).json(*obj);
    }

    static Response close(const Request& req) {
        std::string id = req.getParam("id");
        Domains::Leiloes::Repositories::LeilaoRepository repo(globalPool);
        LOG_DEBUG("[Leiloes::close] id=" + id);
        repo.close(id);
        auto res = Core::Json::makeObject();
        res->asObject()["id"] = Core::Json::makeString(id);
        res->asObject()["status"] = Core::Json::makeString("fechado");
        LOG_INFO("[Leiloes::close] closed id=" + id);
        return Response(StatusCode::OK).json(*res);
    }
};

} // namespace Domains::Leiloes::Controllers


