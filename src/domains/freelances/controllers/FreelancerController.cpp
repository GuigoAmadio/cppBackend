#include "FreelancerController.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include "../../../core/json/Json.hpp"
#include <sstream>

namespace Domains::Freelances::Controllers {

using Core::Http::StatusCode;

FreelancerController::FreelancerController(
    std::shared_ptr<CreateFreelancerUseCase> createUseCase,
    std::shared_ptr<GetFreelancerUseCase> getUseCase,
    std::shared_ptr<UpdateFreelancerUseCase> updateUseCase,
    std::shared_ptr<ListFreelancersUseCase> listUseCase,
    std::shared_ptr<SearchNearbyFreelancersUseCase> searchNearbyUseCase
) : createUseCase_(createUseCase),
    getUseCase_(getUseCase),
    updateUseCase_(updateUseCase),
    listUseCase_(listUseCase),
    searchNearbyUseCase_(searchNearbyUseCase) {}

std::shared_ptr<JsonValue> FreelancerController::freelancerToJson(const FreelancerProfile& freelancer) {
    auto json = Core::Json::makeObject();
    
    json->asObject()["id"] = Core::Json::makeString(freelancer.getId());
    json->asObject()["userId"] = Core::Json::makeString(freelancer.getUserId());
    
    // Especialidades (array)
    auto espArray = Core::Json::makeArray();
    for (const auto& esp : freelancer.getEspecialidades()) {
        espArray->asArray().push_back(Core::Json::makeString(esp));
    }
    json->asObject()["especialidades"] = espArray;
    
    if (freelancer.getExperienciaAnos().has_value()) {
        json->asObject()["experienciaAnos"] = Core::Json::makeNumber(*freelancer.getExperienciaAnos());
    }
    if (freelancer.getPrecoMinimo().has_value()) {
        json->asObject()["precoMinimo"] = Core::Json::makeNumber(*freelancer.getPrecoMinimo());
    }
    if (freelancer.getCpf().has_value()) {
        json->asObject()["cpf"] = Core::Json::makeString(*freelancer.getCpf());
    }
    
    // Coordenadas
    if (freelancer.getCoordenadas().has_value()) {
        auto coords = Core::Json::makeObject();
        coords->asObject()["latitude"] = Core::Json::makeNumber(freelancer.getCoordenadas()->getLatitude());
        coords->asObject()["longitude"] = Core::Json::makeNumber(freelancer.getCoordenadas()->getLongitude());
        json->asObject()["coordenadas"] = coords;
    }
    
    json->asObject()["raioAtuacaoKm"] = Core::Json::makeNumber(freelancer.getRaioAtuacaoKm());
    
    if (freelancer.getDisponibilidade().has_value()) {
        json->asObject()["disponibilidade"] = Core::Json::makeString(*freelancer.getDisponibilidade());
    }
    
    json->asObject()["documentosVerificados"] = Core::Json::makeBool(freelancer.isDocumentosVerificados());
    json->asObject()["ativo"] = Core::Json::makeBool(freelancer.isAtivo());
    json->asObject()["createdAt"] = Core::Json::makeString(freelancer.getCreatedAt());
    json->asObject()["updatedAt"] = Core::Json::makeString(freelancer.getUpdatedAt());
    
    return json;
}

FreelancerProfile FreelancerController::jsonToFreelancer(const std::shared_ptr<JsonValue>& json) {
    auto& obj = json->asObject();
    
    std::string userId = obj["userId"]->asString();
    
    // Parse especialidades array
    std::vector<std::string> especialidades;
    if (obj.find("especialidades") != obj.end() && obj["especialidades"]->isArray()) {
        for (const auto& item : obj["especialidades"]->asArray()) {
            if (item->isString()) {
                especialidades.push_back(item->asString());
            }
        }
    }
    
    FreelancerProfile freelancer("", userId, especialidades);
    
    // Dados opcionais
    if (obj.find("experienciaAnos") != obj.end() && obj["experienciaAnos"]->isNumber()) {
        freelancer.setExperienciaAnos(static_cast<int>(obj["experienciaAnos"]->asNumber()));
    }
    if (obj.find("precoMinimo") != obj.end() && obj["precoMinimo"]->isNumber()) {
        freelancer.setPrecoMinimo(obj["precoMinimo"]->asNumber());
    }
    if (obj.find("cpf") != obj.end() && obj["cpf"]->isString()) {
        freelancer.setCpf(obj["cpf"]->asString());
    }
    if (obj.find("latitude") != obj.end() && obj.find("longitude") != obj.end()) {
        double lat = obj["latitude"]->asNumber();
        double lng = obj["longitude"]->asNumber();
        freelancer.setCoordenadas(ValueObjects::Coordenadas(lat, lng));
    }
    if (obj.find("raioAtuacaoKm") != obj.end() && obj["raioAtuacaoKm"]->isNumber()) {
        freelancer.setRaioAtuacaoKm(obj["raioAtuacaoKm"]->asNumber());
    }
    if (obj.find("disponibilidade") != obj.end() && obj["disponibilidade"]->isString()) {
        freelancer.setDisponibilidade(obj["disponibilidade"]->asString());
    }
    
    return freelancer;
}

Response FreelancerController::create(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_INFO("[FreelancerController] ========== POST /api/freelancers ==========");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            LOG_ERROR("[FreelancerController] Invalid JSON body");
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        LOG_DEBUG("[FreelancerController] JSON parsed successfully");
        
        auto freelancer = jsonToFreelancer(bodyJson);
        
        LOG_DEBUG("[FreelancerController] Calling CreateFreelancerUseCase");
        std::string id = createUseCase_->execute(freelancer);
        
        LOG_INFO("[FreelancerController] Freelancer created successfully - ID: " + id);
        json->asObject()["id"] = Core::Json::makeString(id);
        json->asObject()["message"] = Core::Json::makeString("Freelancer criado com sucesso");
        return Response(StatusCode::Created).json(*json);
        
    } catch (const std::invalid_argument& e) {
        LOG_ERROR("[FreelancerController] Validation error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro de validação: ") + e.what());
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::runtime_error& e) {
        LOG_ERROR("[FreelancerController] Business error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        LOG_ERROR("[FreelancerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro interno: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response FreelancerController::list(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_INFO("[FreelancerController] ========== GET /api/freelancers ==========");
        
        std::string especialidade = req.getQuery("especialidade");
        std::string ativo = req.getQuery("ativo");
        
        std::vector<FreelancerProfile> freelancers;
        
        if (!especialidade.empty()) {
            LOG_DEBUG("[FreelancerController] Filtering by especialidade: " + especialidade);
            freelancers = listUseCase_->executeByEspecialidade(especialidade);
        } else if (ativo == "true") {
            LOG_DEBUG("[FreelancerController] Filtering ativo = true");
            freelancers = listUseCase_->executeAtivos();
        } else {
            LOG_DEBUG("[FreelancerController] No filters - listing all");
            freelancers = listUseCase_->executeAll();
        }
        
        auto array = Core::Json::makeArray();
        for (const auto& freelancer : freelancers) {
            array->asArray().push_back(freelancerToJson(freelancer));
        }
        
        json->asObject()["freelancers"] = array;
        json->asObject()["total"] = Core::Json::makeNumber(static_cast<double>(freelancers.size()));
        
        LOG_INFO("[FreelancerController] Returning " + std::to_string(freelancers.size()) + " freelancers");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[FreelancerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response FreelancerController::getById(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        LOG_INFO("[FreelancerController] ========== GET /api/freelancers/" + id + " ==========");
        
        auto freelancer = getUseCase_->executeById(id);
        
        if (!freelancer.has_value()) {
            LOG_WARNING("[FreelancerController] Freelancer not found");
            json->asObject()["error"] = Core::Json::makeString("Freelancer não encontrado");
            return Response(StatusCode::NotFound).json(*json);
        }
        
        LOG_INFO("[FreelancerController] Freelancer found");
        return Response(StatusCode::OK).json(*freelancerToJson(*freelancer));
        
    } catch (const std::exception& e) {
        LOG_ERROR("[FreelancerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response FreelancerController::getByUserId(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string userId = req.getParam("userId");
        LOG_INFO("[FreelancerController] ========== GET /api/freelancers/user/" + userId + " ==========");
        
        auto freelancer = getUseCase_->executeByUserId(userId);
        
        if (!freelancer.has_value()) {
            LOG_WARNING("[FreelancerController] Freelancer not found for user");
            json->asObject()["error"] = Core::Json::makeString("Freelancer não encontrado para este usuário");
            return Response(StatusCode::NotFound).json(*json);
        }
        
        LOG_INFO("[FreelancerController] Freelancer found for user");
        return Response(StatusCode::OK).json(*freelancerToJson(*freelancer));
        
    } catch (const std::exception& e) {
        LOG_ERROR("[FreelancerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response FreelancerController::update(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        LOG_INFO("[FreelancerController] ========== PUT /api/freelancers/" + id + " ==========");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            LOG_ERROR("[FreelancerController] Invalid JSON body");
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto freelancer = jsonToFreelancer(bodyJson);
        freelancer.setId(id);
        
        LOG_DEBUG("[FreelancerController] Calling UpdateFreelancerUseCase");
        bool success = updateUseCase_->execute(freelancer);
        
        if (!success) {
            LOG_ERROR("[FreelancerController] Failed to update freelancer");
            json->asObject()["error"] = Core::Json::makeString("Falha ao atualizar freelancer");
            return Response(StatusCode::InternalServerError).json(*json);
        }
        
        LOG_INFO("[FreelancerController] Freelancer updated successfully");
        json->asObject()["message"] = Core::Json::makeString("Freelancer atualizado com sucesso");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::runtime_error& e) {
        LOG_ERROR("[FreelancerController] Business error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        LOG_ERROR("[FreelancerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response FreelancerController::deleteById(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        LOG_INFO("[FreelancerController] ========== DELETE /api/freelancers/" + id + " ==========");
        
        json->asObject()["error"] = Core::Json::makeString("Delete não implementado ainda");
        return Response(StatusCode::NotImplemented).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[FreelancerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response FreelancerController::nearby(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_INFO("[FreelancerController] ========== GET /api/freelancers/nearby ==========");
        
        std::string latStr = req.getQuery("lat");
        std::string lngStr = req.getQuery("lng");
        std::string radiusStr = req.getQuery("radius");
        
        if (latStr.empty() || lngStr.empty()) {
            LOG_ERROR("[FreelancerController] Missing lat or lng parameter");
            json->asObject()["error"] = Core::Json::makeString("Parâmetros 'lat' e 'lng' são obrigatórios");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        double latitude = std::stod(latStr);
        double longitude = std::stod(lngStr);
        double radiusKm = radiusStr.empty() ? 10.0 : std::stod(radiusStr);
        
        LOG_DEBUG("[FreelancerController] Searching near: lat=" + latStr + 
                  ", lng=" + lngStr + ", radius=" + std::to_string(radiusKm) + "km");
        
        auto freelancers = searchNearbyUseCase_->execute(latitude, longitude, radiusKm);
        
        auto array = Core::Json::makeArray();
        for (const auto& freelancer : freelancers) {
            array->asArray().push_back(freelancerToJson(freelancer));
        }
        
        json->asObject()["freelancers"] = array;
        json->asObject()["total"] = Core::Json::makeNumber(static_cast<double>(freelancers.size()));
        
        LOG_INFO("[FreelancerController] Found " + std::to_string(freelancers.size()) + " freelancers nearby");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::invalid_argument& e) {
        LOG_ERROR("[FreelancerController] Invalid parameter: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString("Parâmetros inválidos");
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        LOG_ERROR("[FreelancerController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

} // namespace Domains::Freelances::Controllers

