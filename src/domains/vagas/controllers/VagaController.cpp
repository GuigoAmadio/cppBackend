#include "VagaController.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include "../../../core/json/Json.hpp"

namespace Domains::Vagas::Controllers {

using Core::Http::StatusCode;

VagaController::VagaController(
    std::shared_ptr<CreateVagaUseCase> createUseCase,
    std::shared_ptr<GetVagaUseCase> getUseCase,
    std::shared_ptr<UpdateVagaUseCase> updateUseCase,
    std::shared_ptr<ListVagasUseCase> listUseCase
) : createUseCase_(createUseCase), getUseCase_(getUseCase),
    updateUseCase_(updateUseCase), listUseCase_(listUseCase) {}

std::shared_ptr<JsonValue> VagaController::vagaToJson(const Vaga& vaga) {
    auto json = Core::Json::makeObject();
    json->asObject()["id"] = Core::Json::makeString(vaga.getId());
    json->asObject()["restauranteId"] = Core::Json::makeString(vaga.getRestauranteId());
    json->asObject()["titulo"] = Core::Json::makeString(vaga.getTitulo());
    if (vaga.getDescricao().has_value()) json->asObject()["descricao"] = Core::Json::makeString(*vaga.getDescricao());
    json->asObject()["categoria"] = Core::Json::makeString(vaga.getCategoria());
    json->asObject()["precoOferecido"] = Core::Json::makeNumber(vaga.getPrecoOferecido());
    json->asObject()["dataTrabalho"] = Core::Json::makeString(vaga.getDataTrabalho());
    if (vaga.getHorarioInicio().has_value()) json->asObject()["horarioInicio"] = Core::Json::makeString(*vaga.getHorarioInicio());
    if (vaga.getHorarioFim().has_value()) json->asObject()["horarioFim"] = Core::Json::makeString(*vaga.getHorarioFim());
    json->asObject()["vagasDisponiveis"] = Core::Json::makeNumber(vaga.getVagasDisponiveis());
    json->asObject()["vagasPreenchidas"] = Core::Json::makeNumber(vaga.getVagasPreenchidas());
    json->asObject()["status"] = Core::Json::makeString(vaga.getStatus());
    json->asObject()["createdAt"] = Core::Json::makeString(vaga.getCreatedAt());
    return json;
}

Vaga VagaController::jsonToVaga(const std::shared_ptr<JsonValue>& json) {
    auto& obj = json->asObject();
    std::string restauranteId = obj["restauranteId"]->asString();
    std::string titulo = obj["titulo"]->asString();
    std::string categoria = obj["categoria"]->asString();
    double precoOferecido = obj["precoOferecido"]->asNumber();
    std::string dataTrabalho = obj["dataTrabalho"]->asString();
    
    Vaga vaga("", restauranteId, titulo, categoria, precoOferecido, dataTrabalho);
    
    if (obj.find("descricao") != obj.end() && obj["descricao"]->isString()) {
        vaga.setDescricao(obj["descricao"]->asString());
    }
    if (obj.find("horarioInicio") != obj.end() && obj["horarioInicio"]->isString()) {
        vaga.setHorarioInicio(obj["horarioInicio"]->asString());
    }
    if (obj.find("horarioFim") != obj.end() && obj["horarioFim"]->isString()) {
        vaga.setHorarioFim(obj["horarioFim"]->asString());
    }
    if (obj.find("duracaoHoras") != obj.end() && obj["duracaoHoras"]->isNumber()) {
        vaga.setDuracaoHoras(obj["duracaoHoras"]->asNumber());
    }
    if (obj.find("vagasDisponiveis") != obj.end() && obj["vagasDisponiveis"]->isNumber()) {
        vaga.setVagasDisponiveis(static_cast<int>(obj["vagasDisponiveis"]->asNumber()));
    }
    
    return vaga;
}

Response VagaController::create(const Request& req) {
    auto json = Core::Json::makeObject();
    try {
        LOG_INFO("[VagaController] ========== POST /api/vagas ==========");
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto vaga = jsonToVaga(bodyJson);
        std::string id = createUseCase_->execute(vaga);
        
        LOG_INFO("[VagaController] Vaga created - ID: " + id);
        json->asObject()["id"] = Core::Json::makeString(id);
        json->asObject()["message"] = Core::Json::makeString("Vaga criada com sucesso");
        return Response(StatusCode::Created).json(*json);
    } catch (const std::exception& e) {
        LOG_ERROR("[VagaController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response VagaController::list(const Request& req) {
    auto json = Core::Json::makeObject();
    try {
        LOG_INFO("[VagaController] ========== GET /api/vagas ==========");
        
        std::string categoria = req.getQuery("categoria");
        std::string restauranteId = req.getQuery("restauranteId");
        std::string status = req.getQuery("status");
        
        std::vector<Vaga> vagas;
        if (!categoria.empty()) {
            vagas = listUseCase_->executeByCategoria(categoria);
        } else if (!restauranteId.empty()) {
            vagas = listUseCase_->executeByRestaurante(restauranteId);
        } else if (status == "aberta") {
            vagas = listUseCase_->executeAbertas();
        } else {
            vagas = listUseCase_->executeAll();
        }
        
        auto array = Core::Json::makeArray();
        for (const auto& vaga : vagas) {
            array->asArray().push_back(vagaToJson(vaga));
        }
        
        json->asObject()["vagas"] = array;
        json->asObject()["total"] = Core::Json::makeNumber(static_cast<double>(vagas.size()));
        
        LOG_INFO("[VagaController] Returning " + std::to_string(vagas.size()) + " vagas");
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        LOG_ERROR("[VagaController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response VagaController::getById(const Request& req) {
    auto json = Core::Json::makeObject();
    try {
        std::string id = req.getParam("id");
        LOG_INFO("[VagaController] ========== GET /api/vagas/" + id + " ==========");
        
        auto vaga = getUseCase_->executeById(id);
        if (!vaga.has_value()) {
            json->asObject()["error"] = Core::Json::makeString("Vaga não encontrada");
            return Response(StatusCode::NotFound).json(*json);
        }
        
        return Response(StatusCode::OK).json(*vagaToJson(*vaga));
    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response VagaController::update(const Request& req) {
    auto json = Core::Json::makeObject();
    try {
        std::string id = req.getParam("id");
        LOG_INFO("[VagaController] ========== PUT /api/vagas/" + id + " ==========");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto vaga = jsonToVaga(bodyJson);
        vaga.setId(id);
        updateUseCase_->execute(vaga);
        
        json->asObject()["message"] = Core::Json::makeString("Vaga atualizada com sucesso");
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    }
}

Response VagaController::deleteById(const Request& req) {
    auto json = Core::Json::makeObject();
    json->asObject()["error"] = Core::Json::makeString("Delete não implementado");
    return Response(StatusCode::NotImplemented).json(*json);
}

Response VagaController::close(const Request& req) {
    auto json = Core::Json::makeObject();
    try {
        std::string id = req.getParam("id");
        LOG_INFO("[VagaController] ========== POST /api/vagas/" + id + "/close ==========");
        
        auto vagaOpt = getUseCase_->executeById(id);
        if (!vagaOpt.has_value()) {
            json->asObject()["error"] = Core::Json::makeString("Vaga não encontrada");
            return Response(StatusCode::NotFound).json(*json);
        }
        
        Vaga vaga = *vagaOpt;
        vaga.fechar();
        updateUseCase_->execute(vaga);
        
        json->asObject()["message"] = Core::Json::makeString("Vaga fechada com sucesso");
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

} // namespace Domains::Vagas::Controllers

