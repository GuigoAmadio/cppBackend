#pragma once

#include "../../../../src/core/http/Request.hpp"
#include "../../../../src/core/http/Response.hpp"
#include "../repositories/OfferRepository.hpp"
#include "../use_cases/ShortlistOfferUseCase.hpp"
#include "../use_cases/AcceptOfferUseCase.hpp"
#include "../use_cases/CreateOfferUseCase.hpp"
#include "../../leiloes/repositories/LeilaoRepository.hpp"
#include "../../../../src/core/database/ConnectionPool.hpp"
#include "../../../../src/core/database/GlobalPool.hpp"
#include "../../../../src/core/utils/LoggerNew.hpp"

namespace Domains::Offers::Controllers {

using Core::Http::Request;
using Core::Http::Response;
using Core::Http::StatusCode;

class OfferController {
public:
    static Response create(const Request& req) {
        LOG_DEBUG(std::string("[Offers::create] body=") + req.getBody());
        auto body = req.getJson();
        if (!body || !body->isObject()) {
            auto err = Core::Json::makeObject(); err->asObject()["error"] = Core::Json::makeString("Invalid JSON");
            LOG_WARNING("[Offers::create] invalid JSON");
            return Response(StatusCode::BadRequest).json(*err);
        }
        std::optional<std::string> leilaoId;
        auto v = body->get("leilao_id"); if (v && v->isString()) leilaoId = v->asString();

        Domains::Offers::Repositories::OfferRepository offerRepo(globalPool);
        Domains::Leiloes::Repositories::LeilaoRepository leilaoRepo(globalPool);

        // Validar leilao e obter restaurante do leilao (sem fallback)
        std::string restauranteIdFinal;
        if (leilaoId.has_value()) {
            auto l = leilaoRepo.findById(leilaoId.value());
            if (!l.has_value()) {
                auto err = Core::Json::makeObject(); err->asObject()["message"] = Core::Json::makeString("Leilao não encontrado");
                err->asObject()["path"] = Core::Json::makeString("/offers"); err->asObject()["status"] = Core::Json::makeString("error");
                LOG_WARNING("[Offers::create] leilao not found id=" + leilaoId.value());
                return Response(StatusCode::BadRequest).json(*err);
            }
            restauranteIdFinal = l->getRestauranteId();
        }

        // Validar freelancer (sem fallback)
        std::string freelancerIdFinal = body->get("user_id") ? body->get("user_id")->asString() : std::string("");
        try {
            auto conn = globalPool->acquire();
            auto r = conn->executeParams("SELECT 1 FROM freelancer_profiles WHERE id = $1 LIMIT 1", { freelancerIdFinal });
            if (!(r.isSuccess() && r.rowCount() > 0)) {
                auto err = Core::Json::makeObject(); err->asObject()["message"] = Core::Json::makeString("freelancer_id inválido ou inexistente");
                err->asObject()["path"] = Core::Json::makeString("/offers"); err->asObject()["status"] = Core::Json::makeString("error");
                LOG_WARNING("[Offers::create] freelancer_id not found: " + freelancerIdFinal);
                return Response(StatusCode::BadRequest).json(*err);
            }
        } catch (...) {}

        Domains::Offers::UseCases::CreateOfferInput input{
            /*userId*/ freelancerIdFinal,
            /*restId*/ restauranteIdFinal,
            /*leilao*/ leilaoId,
            /*oferecido*/ body->get("valor_oferecido")->asNumber(),
            /*minimo*/ body->get("valor_minimo_aceito")->asNumber(),
            /*msg*/ body->get("mensagem") ? body->get("mensagem")->asString() : std::string("")
        };

        Domains::Offers::UseCases::CreateOfferUseCase uc(&offerRepo, &leilaoRepo);
        LOG_DEBUG("[Offers::create] executing use case");
        auto offer = uc.execute(input);
        auto res = Core::Json::makeObject();
        res->asObject()["id"] = Core::Json::makeString(offer.getId());
        res->asObject()["status"] = Core::Json::makeString(offer.getStatus());
        LOG_INFO("[Offers::create] created id=" + offer.getId());
        return Response(StatusCode::OK).json(*res);
    }

    static Response listForLeilao(const Request& req) {
        std::string id = req.getParam("id");
        Domains::Offers::Repositories::OfferRepository repo(globalPool);
        LOG_DEBUG("[Offers::listForLeilao] leilao_id=" + id);
        auto list = repo.listForLeilaoAsProfiles(id);
        auto arr = Core::Json::makeArray();
        for (const auto& p : list) {
            auto obj = Core::Json::makeObject();
            obj->asObject()["offer_id"] = Core::Json::makeString(p.offerId);
            obj->asObject()["freelancer_id"] = Core::Json::makeString(p.freelancerId);
            obj->asObject()["nome"] = Core::Json::makeString(p.nome);
            obj->asObject()["score"] = Core::Json::makeNumber(p.score);
            obj->asObject()["avaliacoes"] = Core::Json::makeNumber(p.numAvaliacoes);
            obj->asObject()["distancia_km"] = Core::Json::makeNumber(p.distanciaKm);
            obj->asObject()["shortlisted"] = Core::Json::makeBool(p.shortlisted);
            arr->asArray().push_back(obj);
        }
        LOG_INFO("[Offers::listForLeilao] count=" + std::to_string(list.size()));
        return Response(StatusCode::OK).json(*arr);
    }

    static Response setShortlist(const Request& req) {
        auto json = req.getJson();
        bool shortlisted = true;
        if (json && json->isObject()) {
            auto v = json->get("shortlisted");
            if (v && v->isBool()) shortlisted = v->asBool();
        }
        std::string offerId = req.getParam("id");

        auto repo = Domains::Offers::Repositories::OfferRepository(globalPool);
        Domains::Offers::UseCases::ShortlistOfferUseCase uc(&repo);
        LOG_DEBUG("[Offers::setShortlist] id=" + offerId + ", shortlisted=" + std::string(shortlisted?"true":"false"));
        uc.execute(offerId, shortlisted);

        auto res = Core::Json::makeObject();
        res->asObject()["offer_id"] = Core::Json::makeString(offerId);
        res->asObject()["shortlisted"] = Core::Json::makeBool(shortlisted);
        return Response(StatusCode::OK).json(*res);
    }

    static Response accept(const Request& req) {
        std::string offerId = req.getParam("id");
        Domains::Offers::Repositories::OfferRepository offerRepo(globalPool);
        Domains::Leiloes::Repositories::LeilaoRepository leilaoRepo(globalPool);
        Domains::Offers::UseCases::AcceptOfferUseCase uc(&leilaoRepo, &offerRepo);
        LOG_DEBUG("[Offers::accept] id=" + offerId);
        auto result = uc.execute(offerId);

        auto res = Core::Json::makeObject();
        res->asObject()["job_id"] = Core::Json::makeString(result.jobId);
        res->asObject()["freelancer_id"] = Core::Json::makeString(result.freelancerId);
        res->asObject()["valor_final"] = Core::Json::makeNumber(result.valorFinal);
        res->asObject()["status"] = Core::Json::makeString("confirmado");
        LOG_INFO("[Offers::accept] job=" + result.jobId + ", freelancer=" + result.freelancerId);
        return Response(StatusCode::OK).json(*res);
    }
};

} // namespace Domains::Offers::Controllers


