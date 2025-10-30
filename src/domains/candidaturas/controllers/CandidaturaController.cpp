#include "CandidaturaController.hpp"
#include "../../../core/json/Json.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>

using Core::Http::StatusCode;

namespace Domains {
namespace Candidaturas {
namespace Controllers {

CandidaturaController::CandidaturaController(
    std::shared_ptr<CandidaturaRepository> repo,
    std::shared_ptr<VagaRepository> vagaRepo
) : repo_(repo), vagaRepo_(vagaRepo) {
    createUseCase_ = std::make_shared<CreateCandidaturaUseCase>(repo_, vagaRepo_);
    acceptUseCase_ = std::make_shared<AcceptCandidaturaUseCase>(repo_, vagaRepo_);
    rejectUseCase_ = std::make_shared<RejectCandidaturaUseCase>(repo_);
}

Response CandidaturaController::create(const Request& req) {
    LOG_INFO("[CandidaturaController] ========== POST /api/candidaturas ==========");
    
    auto json = Core::Json::makeObject();
    
    try {
        // Parse JSON
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            LOG_ERROR("[CandidaturaController] Invalid JSON body");
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto obj = bodyJson->asObject();
        LOG_DEBUG("[CandidaturaController] JSON parsed successfully");
        
        // Extrair campos
        std::string vagaId = obj.count("vagaId") ? obj["vagaId"]->asString() : "";
        std::string freelancerId = obj.count("freelancerId") ? obj["freelancerId"]->asString() : "";
        
        std::optional<double> precoPropostoFreelancer;
        if (obj.count("precoPropostoFreelancer")) {
            precoPropostoFreelancer = obj["precoPropostoFreelancer"]->asNumber();
        }
        
        std::optional<std::string> mensagem;
        if (obj.count("mensagem")) {
            mensagem = obj["mensagem"]->asString();
        }
        
        // Criar candidatura
        Entities::Candidatura candidatura(
            "", vagaId, freelancerId, precoPropostoFreelancer,
            mensagem, "pendente", std::nullopt, "", std::nullopt
        );
        
        LOG_DEBUG("[CandidaturaController] Calling CreateCandidaturaUseCase");
        std::string id = createUseCase_->execute(candidatura);
        
        LOG_INFO("[CandidaturaController] Candidatura created - ID: " + id);
        
        json->asObject()["id"] = Core::Json::makeString(id);
        json->asObject()["status"] = Core::Json::makeString("pendente");
        json->asObject()["message"] = Core::Json::makeString("Candidatura criada com sucesso");
        
        return Response(StatusCode::Created).json(*json);
        
    } catch (const std::runtime_error& e) {
        LOG_ERROR("[CandidaturaController] Business error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        LOG_ERROR("[CandidaturaController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CandidaturaController::getById(const Request& req) {
    LOG_INFO("[CandidaturaController] ========== GET /api/candidaturas/:id ==========");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        LOG_DEBUG("[CandidaturaController] Looking for candidatura: " + id);
        
        auto candidatura = repo_->findById(id);
        if (!candidatura) {
            LOG_WARNING("[CandidaturaController] Candidatura not found");
            json->asObject()["error"] = Core::Json::makeString("Candidatura not found");
            return Response(StatusCode::NotFound).json(*json);
        }
        
        // Montar JSON
        json->asObject()["id"] = Core::Json::makeString(candidatura->getId());
        json->asObject()["vagaId"] = Core::Json::makeString(candidatura->getVagaId());
        json->asObject()["freelancerId"] = Core::Json::makeString(candidatura->getFreelancerId());
        json->asObject()["status"] = Core::Json::makeString(candidatura->getStatus());
        json->asObject()["createdAt"] = Core::Json::makeString(candidatura->getCreatedAt());
        
        if (candidatura->getPrecoPropostoFreelancer()) {
            json->asObject()["precoPropostoFreelancer"] = Core::Json::makeNumber(*candidatura->getPrecoPropostoFreelancer());
        }
        
        if (candidatura->getMensagem()) {
            json->asObject()["mensagem"] = Core::Json::makeString(*candidatura->getMensagem());
        }
        
        if (candidatura->getMotivoRejeicao()) {
            json->asObject()["motivoRejeicao"] = Core::Json::makeString(*candidatura->getMotivoRejeicao());
        }
        
        if (candidatura->getRespondedAt()) {
            json->asObject()["respondedAt"] = Core::Json::makeString(*candidatura->getRespondedAt());
        }
        
        LOG_INFO("[CandidaturaController] Candidatura found");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[CandidaturaController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CandidaturaController::getByVaga(const Request& req) {
    LOG_INFO("[CandidaturaController] ========== GET /api/candidaturas/vaga/:vagaId ==========");
    
    try {
        std::string vagaId = req.getParam("vagaId");
        LOG_DEBUG("[CandidaturaController] Looking for candidaturas of vaga: " + vagaId);
        
        auto candidaturas = repo_->findByVaga(vagaId);
        
        // Montar JSON array
        auto jsonArray = Core::Json::makeArray();
        for (const auto& c : candidaturas) {
            auto item = Core::Json::makeObject();
            item->asObject()["id"] = Core::Json::makeString(c.getId());
            item->asObject()["vagaId"] = Core::Json::makeString(c.getVagaId());
            item->asObject()["freelancerId"] = Core::Json::makeString(c.getFreelancerId());
            item->asObject()["status"] = Core::Json::makeString(c.getStatus());
            item->asObject()["createdAt"] = Core::Json::makeString(c.getCreatedAt());
            
            if (c.getPrecoPropostoFreelancer()) {
                item->asObject()["precoPropostoFreelancer"] = Core::Json::makeNumber(*c.getPrecoPropostoFreelancer());
            }
            
            jsonArray->asArray().push_back(item);
        }
        
        LOG_INFO("[CandidaturaController] Returning " + std::to_string(candidaturas.size()) + " candidaturas");
        return Response(StatusCode::OK).json(*jsonArray);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[CandidaturaController] Exception: " + std::string(e.what()));
        auto json = Core::Json::makeObject();
        json->asObject()["error"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CandidaturaController::getByFreelancer(const Request& req) {
    LOG_INFO("[CandidaturaController] ========== GET /api/candidaturas/freelancer/:freelancerId ==========");
    
    try {
        std::string freelancerId = req.getParam("freelancerId");
        LOG_DEBUG("[CandidaturaController] Looking for candidaturas of freelancer: " + freelancerId);
        
        auto candidaturas = repo_->findByFreelancer(freelancerId);
        
        // Montar JSON array
        auto jsonArray = Core::Json::makeArray();
        for (const auto& c : candidaturas) {
            auto item = Core::Json::makeObject();
            item->asObject()["id"] = Core::Json::makeString(c.getId());
            item->asObject()["vagaId"] = Core::Json::makeString(c.getVagaId());
            item->asObject()["freelancerId"] = Core::Json::makeString(c.getFreelancerId());
            item->asObject()["status"] = Core::Json::makeString(c.getStatus());
            item->asObject()["createdAt"] = Core::Json::makeString(c.getCreatedAt());
            
            if (c.getPrecoPropostoFreelancer()) {
                item->asObject()["precoPropostoFreelancer"] = Core::Json::makeNumber(*c.getPrecoPropostoFreelancer());
            }
            
            jsonArray->asArray().push_back(item);
        }
        
        LOG_INFO("[CandidaturaController] Returning " + std::to_string(candidaturas.size()) + " candidaturas");
        return Response(StatusCode::OK).json(*jsonArray);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[CandidaturaController] Exception: " + std::string(e.what()));
        auto json = Core::Json::makeObject();
        json->asObject()["error"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CandidaturaController::accept(const Request& req) {
    LOG_INFO("[CandidaturaController] ========== PUT /api/candidaturas/:id/accept ==========");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        LOG_DEBUG("[CandidaturaController] Accepting candidatura: " + id);
        
        acceptUseCase_->execute(id);
        
        LOG_INFO("[CandidaturaController] Candidatura accepted successfully");
        json->asObject()["message"] = Core::Json::makeString("Candidatura aceita com sucesso");
        json->asObject()["status"] = Core::Json::makeString("aceita");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::runtime_error& e) {
        LOG_ERROR("[CandidaturaController] Business error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        LOG_ERROR("[CandidaturaController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response CandidaturaController::reject(const Request& req) {
    LOG_INFO("[CandidaturaController] ========== PUT /api/candidaturas/:id/reject ==========");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        
        // Parse JSON body
        auto bodyJson = req.getJson();
        std::string motivo = "Sem motivo especificado";
        if (bodyJson && bodyJson->isObject()) {
            auto obj = bodyJson->asObject();
            if (obj.count("motivo")) {
                motivo = obj["motivo"]->asString();
            }
        }
        
        LOG_DEBUG("[CandidaturaController] Rejecting candidatura: " + id);
        
        rejectUseCase_->execute(id, motivo);
        
        LOG_INFO("[CandidaturaController] Candidatura rejected successfully");
        json->asObject()["message"] = Core::Json::makeString("Candidatura rejeitada");
        json->asObject()["status"] = Core::Json::makeString("rejeitada");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::runtime_error& e) {
        LOG_ERROR("[CandidaturaController] Business error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        LOG_ERROR("[CandidaturaController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString("Internal server error");
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

} // namespace Controllers
} // namespace Candidaturas
} // namespace Domains
