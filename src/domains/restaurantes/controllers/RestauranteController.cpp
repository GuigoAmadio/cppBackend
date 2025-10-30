#include "RestauranteController.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include "../../../core/json/Json.hpp"
#include <sstream>

namespace Domains::Restaurantes::Controllers {

using Core::Http::StatusCode;

RestauranteController::RestauranteController(
    std::shared_ptr<CreateRestauranteUseCase> createUseCase,
    std::shared_ptr<GetRestauranteUseCase> getUseCase,
    std::shared_ptr<UpdateRestauranteUseCase> updateUseCase,
    std::shared_ptr<ListRestaurantesUseCase> listUseCase,
    std::shared_ptr<SearchNearbyRestaurantesUseCase> searchNearbyUseCase
) : createUseCase_(createUseCase),
    getUseCase_(getUseCase),
    updateUseCase_(updateUseCase),
    listUseCase_(listUseCase),
    searchNearbyUseCase_(searchNearbyUseCase) {}

std::shared_ptr<JsonValue> RestauranteController::restauranteToJson(const RestauranteProfile& rest) {
    auto json = Core::Json::makeObject();
    
    json->asObject()["id"] = Core::Json::makeString(rest.getId());
    json->asObject()["userId"] = Core::Json::makeString(rest.getUserId());
    json->asObject()["cnpj"] = Core::Json::makeString(rest.getCnpj());
    json->asObject()["razaoSocial"] = Core::Json::makeString(rest.getRazaoSocial());
    
    if (rest.getNomeFantasia().has_value()) {
        json->asObject()["nomeFantasia"] = Core::Json::makeString(*rest.getNomeFantasia());
    }
    if (rest.getDescricao().has_value()) {
        json->asObject()["descricao"] = Core::Json::makeString(*rest.getDescricao());
    }
    if (rest.getCategoria().has_value()) {
        json->asObject()["categoria"] = Core::Json::makeString(*rest.getCategoria());
    }
    if (rest.getLogo().has_value()) {
        json->asObject()["logo"] = Core::Json::makeString(*rest.getLogo());
    }
    
    // Coordenadas
    auto coords = Core::Json::makeObject();
    coords->asObject()["latitude"] = Core::Json::makeNumber(rest.getCoordenadas().getLatitude());
    coords->asObject()["longitude"] = Core::Json::makeNumber(rest.getCoordenadas().getLongitude());
    json->asObject()["coordenadas"] = coords;
    
    if (rest.getEnderecoCompleto().has_value()) {
        json->asObject()["enderecoCompleto"] = Core::Json::makeString(*rest.getEnderecoCompleto());
    }
    if (rest.getCep().has_value()) {
        json->asObject()["cep"] = Core::Json::makeString(*rest.getCep());
    }
    if (rest.getCidade().has_value()) {
        json->asObject()["cidade"] = Core::Json::makeString(*rest.getCidade());
    }
    if (rest.getEstado().has_value()) {
        json->asObject()["estado"] = Core::Json::makeString(*rest.getEstado());
    }
    if (rest.getHorarioFuncionamento().has_value()) {
        json->asObject()["horarioFuncionamento"] = Core::Json::makeString(*rest.getHorarioFuncionamento());
    }
    
    json->asObject()["documentosVerificados"] = Core::Json::makeBool(rest.isDocumentosVerificados());
    json->asObject()["ativo"] = Core::Json::makeBool(rest.isAtivo());
    json->asObject()["createdAt"] = Core::Json::makeString(rest.getCreatedAt());
    json->asObject()["updatedAt"] = Core::Json::makeString(rest.getUpdatedAt());
    
    return json;
}

RestauranteProfile RestauranteController::jsonToRestaurante(const std::shared_ptr<JsonValue>& json) {
    auto& obj = json->asObject();
    
    std::string userId = obj["userId"]->asString();
    std::string cnpj = obj["cnpj"]->asString();
    std::string razaoSocial = obj["razaoSocial"]->asString();
    double latitude = obj["latitude"]->asNumber();
    double longitude = obj["longitude"]->asNumber();
    
    // Criar entity com dados obrigatórios
    RestauranteProfile restaurante("", userId, cnpj, razaoSocial, latitude, longitude);
    
    // Dados opcionais
    if (obj.find("nomeFantasia") != obj.end() && obj["nomeFantasia"]->isString()) {
        restaurante.setNomeFantasia(obj["nomeFantasia"]->asString());
    }
    if (obj.find("descricao") != obj.end() && obj["descricao"]->isString()) {
        restaurante.setDescricao(obj["descricao"]->asString());
    }
    if (obj.find("categoria") != obj.end() && obj["categoria"]->isString()) {
        restaurante.setCategoria(obj["categoria"]->asString());
    }
    if (obj.find("logo") != obj.end() && obj["logo"]->isString()) {
        restaurante.setLogo(obj["logo"]->asString());
    }
    if (obj.find("enderecoCompleto") != obj.end() && obj["enderecoCompleto"]->isString()) {
        restaurante.setEnderecoCompleto(obj["enderecoCompleto"]->asString());
    }
    if (obj.find("cep") != obj.end() && obj["cep"]->isString()) {
        restaurante.setCep(obj["cep"]->asString());
    }
    if (obj.find("cidade") != obj.end() && obj["cidade"]->isString()) {
        restaurante.setCidade(obj["cidade"]->asString());
    }
    if (obj.find("estado") != obj.end() && obj["estado"]->isString()) {
        restaurante.setEstado(obj["estado"]->asString());
    }
    if (obj.find("horarioFuncionamento") != obj.end() && obj["horarioFuncionamento"]->isString()) {
        restaurante.setHorarioFuncionamento(obj["horarioFuncionamento"]->asString());
    }
    
    return restaurante;
}

Response RestauranteController::create(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_INFO("[RestauranteController] ========== POST /api/restaurantes ==========");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            LOG_ERROR("[RestauranteController] Invalid JSON body");
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        LOG_DEBUG("[RestauranteController] JSON parsed successfully");
        
        // Criar entity a partir do JSON
        auto restaurante = jsonToRestaurante(bodyJson);
        
        // Executar use case
        LOG_DEBUG("[RestauranteController] Calling CreateRestauranteUseCase");
        std::string id = createUseCase_->execute(restaurante);
        
        LOG_INFO("[RestauranteController] Restaurante created successfully - ID: " + id);
        json->asObject()["id"] = Core::Json::makeString(id);
        json->asObject()["message"] = Core::Json::makeString("Restaurante criado com sucesso");
        return Response(StatusCode::Created).json(*json);
        
    } catch (const std::invalid_argument& e) {
        LOG_ERROR("[RestauranteController] Validation error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro de validação: ") + e.what());
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::runtime_error& e) {
        LOG_ERROR("[RestauranteController] Business error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        LOG_ERROR("[RestauranteController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro interno: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response RestauranteController::list(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_INFO("[RestauranteController] ========== GET /api/restaurantes ==========");
        
        // Verificar query params para filtros
        std::string categoria = req.getQuery("categoria");
        std::string cidade = req.getQuery("cidade");
        std::string ativo = req.getQuery("ativo");
        
        std::vector<RestauranteProfile> restaurantes;
        
        if (!categoria.empty()) {
            LOG_DEBUG("[RestauranteController] Filtering by categoria: " + categoria);
            restaurantes = listUseCase_->executeByCategoria(categoria);
        } else if (!cidade.empty()) {
            LOG_DEBUG("[RestauranteController] Filtering by cidade: " + cidade);
            restaurantes = listUseCase_->executeByCidade(cidade);
        } else if (ativo == "true") {
            LOG_DEBUG("[RestauranteController] Filtering ativo = true");
            restaurantes = listUseCase_->executeAtivos();
        } else {
            LOG_DEBUG("[RestauranteController] No filters - listing all");
            restaurantes = listUseCase_->executeAll();
        }
        
        // Montar array JSON
        auto array = Core::Json::makeArray();
        for (const auto& rest : restaurantes) {
            array->asArray().push_back(restauranteToJson(rest));
        }
        
        json->asObject()["restaurantes"] = array;
        json->asObject()["total"] = Core::Json::makeNumber(static_cast<double>(restaurantes.size()));
        
        LOG_INFO("[RestauranteController] Returning " + std::to_string(restaurantes.size()) + " restaurantes");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[RestauranteController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response RestauranteController::getById(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        LOG_INFO("[RestauranteController] ========== GET /api/restaurantes/" + id + " ==========");
        
        auto restaurante = getUseCase_->executeById(id);
        
        if (!restaurante.has_value()) {
            LOG_WARNING("[RestauranteController] Restaurante not found");
            json->asObject()["error"] = Core::Json::makeString("Restaurante não encontrado");
            return Response(StatusCode::NotFound).json(*json);
        }
        
        LOG_INFO("[RestauranteController] Restaurante found");
        return Response(StatusCode::OK).json(*restauranteToJson(*restaurante));
        
    } catch (const std::exception& e) {
        LOG_ERROR("[RestauranteController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response RestauranteController::getByUserId(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string userId = req.getParam("userId");
        LOG_INFO("[RestauranteController] ========== GET /api/restaurantes/user/" + userId + " ==========");
        
        auto restaurante = getUseCase_->executeByUserId(userId);
        
        if (!restaurante.has_value()) {
            LOG_WARNING("[RestauranteController] Restaurante not found for user");
            json->asObject()["error"] = Core::Json::makeString("Restaurante não encontrado para este usuário");
            return Response(StatusCode::NotFound).json(*json);
        }
        
        LOG_INFO("[RestauranteController] Restaurante found for user");
        return Response(StatusCode::OK).json(*restauranteToJson(*restaurante));
        
    } catch (const std::exception& e) {
        LOG_ERROR("[RestauranteController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response RestauranteController::update(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        LOG_INFO("[RestauranteController] ========== PUT /api/restaurantes/" + id + " ==========");
        
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            LOG_ERROR("[RestauranteController] Invalid JSON body");
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        // Criar entity com ID
        auto restaurante = jsonToRestaurante(bodyJson);
        restaurante.setId(id);
        
        // Executar use case
        LOG_DEBUG("[RestauranteController] Calling UpdateRestauranteUseCase");
        bool success = updateUseCase_->execute(restaurante);
        
        if (!success) {
            LOG_ERROR("[RestauranteController] Failed to update restaurante");
            json->asObject()["error"] = Core::Json::makeString("Falha ao atualizar restaurante");
            return Response(StatusCode::InternalServerError).json(*json);
        }
        
        LOG_INFO("[RestauranteController] Restaurante updated successfully");
        json->asObject()["message"] = Core::Json::makeString("Restaurante atualizado com sucesso");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::runtime_error& e) {
        LOG_ERROR("[RestauranteController] Business error: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        LOG_ERROR("[RestauranteController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response RestauranteController::deleteById(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        std::string id = req.getParam("id");
        LOG_INFO("[RestauranteController] ========== DELETE /api/restaurantes/" + id + " ==========");
        
        // TODO: Implementar DeleteRestauranteUseCase se necessário
        // Por enquanto, deletar diretamente via repository seria necessário
        
        json->asObject()["error"] = Core::Json::makeString("Delete não implementado ainda");
        return Response(StatusCode::NotImplemented).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[RestauranteController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response RestauranteController::nearby(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_INFO("[RestauranteController] ========== GET /api/restaurantes/nearby ==========");
        
        // Obter query params
        std::string latStr = req.getQuery("lat");
        std::string lngStr = req.getQuery("lng");
        std::string radiusStr = req.getQuery("radius");
        
        if (latStr.empty() || lngStr.empty()) {
            LOG_ERROR("[RestauranteController] Missing lat or lng parameter");
            json->asObject()["error"] = Core::Json::makeString("Parâmetros 'lat' e 'lng' são obrigatórios");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        double latitude = std::stod(latStr);
        double longitude = std::stod(lngStr);
        double radiusKm = radiusStr.empty() ? 10.0 : std::stod(radiusStr);
        
        LOG_DEBUG("[RestauranteController] Searching near: lat=" + latStr + 
                  ", lng=" + lngStr + ", radius=" + std::to_string(radiusKm) + "km");
        
        auto restaurantes = searchNearbyUseCase_->execute(latitude, longitude, radiusKm);
        
        // Montar array JSON
        auto array = Core::Json::makeArray();
        for (const auto& rest : restaurantes) {
            array->asArray().push_back(restauranteToJson(rest));
        }
        
        json->asObject()["restaurantes"] = array;
        json->asObject()["total"] = Core::Json::makeNumber(static_cast<double>(restaurantes.size()));
        
        LOG_INFO("[RestauranteController] Found " + std::to_string(restaurantes.size()) + " restaurantes nearby");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::invalid_argument& e) {
        LOG_ERROR("[RestauranteController] Invalid parameter: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString("Parâmetros inválidos");
        return Response(StatusCode::BadRequest).json(*json);
    } catch (const std::exception& e) {
        LOG_ERROR("[RestauranteController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

Response RestauranteController::map(const Request& req) {
    auto json = Core::Json::makeObject();
    
    try {
        LOG_INFO("[RestauranteController] ========== GET /api/restaurantes/map ==========");
        
        // Retornar apenas restaurantes ativos (para exibir no mapa)
        auto restaurantes = listUseCase_->executeAtivos();
        
        // Montar array JSON simplificado (apenas dados necessários para o mapa)
        auto array = Core::Json::makeArray();
        for (const auto& rest : restaurantes) {
            auto marker = Core::Json::makeObject();
            marker->asObject()["id"] = Core::Json::makeString(rest.getId());
            marker->asObject()["nome"] = Core::Json::makeString(
                rest.getNomeFantasia().value_or(rest.getRazaoSocial())
            );
            marker->asObject()["categoria"] = Core::Json::makeString(
                rest.getCategoria().value_or("restaurante")
            );
            marker->asObject()["latitude"] = Core::Json::makeNumber(rest.getCoordenadas().getLatitude());
            marker->asObject()["longitude"] = Core::Json::makeNumber(rest.getCoordenadas().getLongitude());
            if (rest.getCidade().has_value()) {
                marker->asObject()["cidade"] = Core::Json::makeString(*rest.getCidade());
            }
            
            array->asArray().push_back(marker);
        }
        
        json->asObject()["markers"] = array;
        json->asObject()["total"] = Core::Json::makeNumber(static_cast<double>(restaurantes.size()));
        
        LOG_INFO("[RestauranteController] Returning " + std::to_string(restaurantes.size()) + " markers for map");
        return Response(StatusCode::OK).json(*json);
        
    } catch (const std::exception& e) {
        LOG_ERROR("[RestauranteController] Exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string("Erro: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

} // namespace Domains::Restaurantes::Controllers

