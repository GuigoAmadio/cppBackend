#include "FreelancerRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include "../../../core/json/JsonParser.hpp"
#include <sstream>
#include <random>
#include <iomanip>

namespace Domains::Freelances::Repositories {

// Função auxiliar para gerar UUID
static std::string generateId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex;
    
    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);
    
    return ss.str();
}

FreelancerProfile FreelancerRepository::mapToFreelancer(
    const QueryResult& result,
    int row
) {
    LOG_DEBUG("[FreelancerRepository] Mapping row " + std::to_string(row) + " to FreelancerProfile");
    
    std::string id = result.getValue(row, 0);
    std::string userId = result.getValue(row, 1);
    std::string especialidadesJson = result.getValue(row, 2);
    
    // Parse especialidades JSON array
    std::vector<std::string> especialidades;
    try {
        auto parser = Core::Json::JsonParser(especialidadesJson);
        auto jsonValue = parser.parse();
        if (jsonValue && jsonValue->isArray()) {
            for (const auto& item : jsonValue->asArray()) {
                if (item->isString()) {
                    especialidades.push_back(item->asString());
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_WARNING("[FreelancerRepository] Failed to parse especialidades JSON: " + std::string(e.what()));
    }
    
    FreelancerProfile freelancer(id, userId, especialidades);
    
    // experiencia_anos (col 3)
    std::string expAnos = result.getValue(row, 3);
    if (!expAnos.empty()) {
        freelancer.setExperienciaAnos(std::stoi(expAnos));
    }
    
    // preco_minimo (col 4)
    std::string precoMin = result.getValue(row, 4);
    if (!precoMin.empty()) {
        freelancer.setPrecoMinimo(std::stod(precoMin));
    }
    
    // cpf (col 5)
    std::string cpf = result.getValue(row, 5);
    if (!cpf.empty()) {
        freelancer.setCpf(cpf);
    }
    
    // latitude (col 6), longitude (col 7)
    std::string latStr = result.getValue(row, 6);
    std::string lonStr = result.getValue(row, 7);
    if (!latStr.empty() && !lonStr.empty()) {
        double lat = std::stod(latStr);
        double lon = std::stod(lonStr);
        freelancer.setCoordenadas(ValueObjects::Coordenadas(lat, lon));
    }
    
    // raio_atuacao_km (col 8)
    std::string raio = result.getValue(row, 8);
    if (!raio.empty()) {
        freelancer.setRaioAtuacaoKm(std::stod(raio));
    }
    
    // disponibilidade (col 9)
    std::string disp = result.getValue(row, 9);
    if (!disp.empty()) {
        freelancer.setDisponibilidade(disp);
    }
    
    // documentos_verificados (col 10)
    std::string docVerif = result.getValue(row, 10);
    if (!docVerif.empty()) {
        freelancer.setDocumentosVerificados(docVerif == "t" || docVerif == "true" || docVerif == "1");
    }
    
    // ativo (col 11)
    std::string ativo = result.getValue(row, 11);
    if (!ativo.empty()) {
        freelancer.setAtivo(ativo == "t" || ativo == "true" || ativo == "1");
    }
    
    // created_at (col 12)
    std::string createdAt = result.getValue(row, 12);
    if (!createdAt.empty()) {
        freelancer.setCreatedAt(createdAt);
    }
    
    // updated_at (col 13)
    std::string updatedAt = result.getValue(row, 13);
    if (!updatedAt.empty()) {
        freelancer.setUpdatedAt(updatedAt);
    }
    
    return freelancer;
}

std::string FreelancerRepository::create(const FreelancerProfile& freelancer) {
    LOG_INFO("[FreelancerRepository] create() - user_id: " + freelancer.getUserId());
    
    auto conn = pool_->acquire();
    LOG_DEBUG("[FreelancerRepository] Connection acquired");
    
    std::string id = generateId();
    LOG_DEBUG("[FreelancerRepository] Generated ID: " + id);
    
    // Converter especialidades para JSON array
    std::string especialidadesJson = "[";
    for (size_t i = 0; i < freelancer.getEspecialidades().size(); i++) {
        if (i > 0) especialidadesJson += ",";
        especialidadesJson += "\"" + freelancer.getEspecialidades()[i] + "\"";
    }
    especialidadesJson += "]";
    LOG_DEBUG("[FreelancerRepository] especialidades JSON: " + especialidadesJson);
    
    std::string query = R"(
        INSERT INTO freelancer_profiles (
            id, user_id, especialidades, experiencia_anos, preco_minimo, cpf,
            latitude, longitude, raio_atuacao_km, disponibilidade,
            documentos_verificados, ativo
        ) VALUES (
            $1, $2, $3::jsonb, NULLIF($4,'')::integer, NULLIF($5,'')::decimal, NULLIF($6,''),
            NULLIF($7,'')::decimal, NULLIF($8,'')::decimal, $9, NULLIF($10,'')::jsonb,
            $11, $12
        )
        RETURNING id
    )";
    
    std::string latStr = "";
    std::string lonStr = "";
    if (freelancer.getCoordenadas().has_value()) {
        latStr = std::to_string(freelancer.getCoordenadas()->getLatitude());
        lonStr = std::to_string(freelancer.getCoordenadas()->getLongitude());
    }
    
    std::vector<std::string> params = {
        id,
        freelancer.getUserId(),
        especialidadesJson,
        freelancer.getExperienciaAnos().has_value() ? std::to_string(*freelancer.getExperienciaAnos()) : "",
        freelancer.getPrecoMinimo().has_value() ? std::to_string(*freelancer.getPrecoMinimo()) : "",
        freelancer.getCpf().value_or(""),
        latStr,
        lonStr,
        std::to_string(freelancer.getRaioAtuacaoKm()),
        freelancer.getDisponibilidade().value_or(""),
        freelancer.isDocumentosVerificados() ? "true" : "false",
        freelancer.isAtivo() ? "true" : "false"
    };
    
    try {
        auto result = conn->executeParams(query, params);
        
        // Verificar se o INSERT foi bem-sucedido
        if (result.rowCount() == 0) {
            LOG_ERROR("[FreelancerRepository] INSERT FAILED! No rows returned from RETURNING clause");
            throw std::runtime_error("INSERT into freelancer_profiles failed - no rows returned");
        }
        
        std::string returnedId = result.getValue(0, 0);
        LOG_INFO("[FreelancerRepository] Freelancer created successfully - ID: " + returnedId);
        return returnedId;
    } catch (const std::exception& e) {
        LOG_ERROR("[FreelancerRepository] Error creating freelancer: " + std::string(e.what()));
        throw;
    }
}

std::optional<FreelancerProfile> FreelancerRepository::findById(const std::string& id) {
    LOG_DEBUG("[FreelancerRepository] findById() - id: " + id);
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM freelancer_profiles WHERE id = $1";
    auto result = conn->executeParams(query, {id});
    
    if (result.rowCount() == 0) {
        LOG_DEBUG("[FreelancerRepository] Freelancer not found");
        return std::nullopt;
    }
    
    LOG_INFO("[FreelancerRepository] Freelancer found - ID: " + id);
    return mapToFreelancer(result, 0);
}

std::optional<FreelancerProfile> FreelancerRepository::findByUserId(const std::string& userId) {
    LOG_DEBUG("[FreelancerRepository] findByUserId() - user_id: " + userId);
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM freelancer_profiles WHERE user_id = $1";
    auto result = conn->executeParams(query, {userId});
    
    if (result.rowCount() == 0) {
        LOG_DEBUG("[FreelancerRepository] Freelancer not found for user");
        return std::nullopt;
    }
    
    LOG_INFO("[FreelancerRepository] Freelancer found for user: " + userId);
    return mapToFreelancer(result, 0);
}

std::optional<FreelancerProfile> FreelancerRepository::findByCpf(const std::string& cpf) {
    LOG_DEBUG("[FreelancerRepository] findByCpf() - cpf: " + cpf);
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM freelancer_profiles WHERE cpf = $1";
    auto result = conn->executeParams(query, {cpf});
    
    if (result.rowCount() == 0) {
        LOG_DEBUG("[FreelancerRepository] Freelancer not found for CPF");
        return std::nullopt;
    }
    
    LOG_INFO("[FreelancerRepository] Freelancer found for CPF: " + cpf);
    return mapToFreelancer(result, 0);
}

std::vector<FreelancerProfile> FreelancerRepository::findAll() {
    LOG_DEBUG("[FreelancerRepository] findAll()");
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM freelancer_profiles ORDER BY created_at DESC";
    auto result = conn->executeParams(query, {});
    
    std::vector<FreelancerProfile> freelancers;
    for (int i = 0; i < result.rowCount(); i++) {
        freelancers.push_back(mapToFreelancer(result, i));
    }
    
    LOG_INFO("[FreelancerRepository] Found " + std::to_string(freelancers.size()) + " freelancers");
    return freelancers;
}

std::vector<FreelancerProfile> FreelancerRepository::findByEspecialidade(const std::string& especialidade) {
    LOG_DEBUG("[FreelancerRepository] findByEspecialidade() - especialidade: " + especialidade);
    
    auto conn = pool_->acquire();
    
    // Usar operador JSONB @> para verificar se array contém elemento
    std::string query = R"(
        SELECT * FROM freelancer_profiles 
        WHERE especialidades @> $1::jsonb 
        ORDER BY created_at DESC
    )";
    
    std::string especialidadeJson = "[\"" + especialidade + "\"]";
    auto result = conn->executeParams(query, {especialidadeJson});
    
    std::vector<FreelancerProfile> freelancers;
    for (int i = 0; i < result.rowCount(); i++) {
        freelancers.push_back(mapToFreelancer(result, i));
    }
    
    LOG_INFO("[FreelancerRepository] Found " + std::to_string(freelancers.size()) + " freelancers for especialidade");
    return freelancers;
}

std::vector<FreelancerProfile> FreelancerRepository::findAtivos() {
    LOG_DEBUG("[FreelancerRepository] findAtivos()");
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM freelancer_profiles WHERE ativo = true ORDER BY created_at DESC";
    auto result = conn->executeParams(query, {});
    
    std::vector<FreelancerProfile> freelancers;
    for (int i = 0; i < result.rowCount(); i++) {
        freelancers.push_back(mapToFreelancer(result, i));
    }
    
    LOG_INFO("[FreelancerRepository] Found " + std::to_string(freelancers.size()) + " freelancers ativos");
    return freelancers;
}

bool FreelancerRepository::update(const FreelancerProfile& freelancer) {
    LOG_INFO("[FreelancerRepository] update() - id: " + freelancer.getId());
    
    auto conn = pool_->acquire();
    
    // Converter especialidades para JSON
    std::string especialidadesJson = "[";
    for (size_t i = 0; i < freelancer.getEspecialidades().size(); i++) {
        if (i > 0) especialidadesJson += ",";
        especialidadesJson += "\"" + freelancer.getEspecialidades()[i] + "\"";
    }
    especialidadesJson += "]";
    
    std::string query = R"(
        UPDATE freelancer_profiles SET
            especialidades = $1::jsonb,
            experiencia_anos = NULLIF($2,'')::integer,
            preco_minimo = NULLIF($3,'')::decimal,
            cpf = NULLIF($4,''),
            latitude = NULLIF($5,'')::decimal,
            longitude = NULLIF($6,'')::decimal,
            raio_atuacao_km = $7,
            disponibilidade = NULLIF($8,'')::jsonb,
            documentos_verificados = $9,
            ativo = $10,
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $11
    )";
    
    std::string latStr = "";
    std::string lonStr = "";
    if (freelancer.getCoordenadas().has_value()) {
        latStr = std::to_string(freelancer.getCoordenadas()->getLatitude());
        lonStr = std::to_string(freelancer.getCoordenadas()->getLongitude());
    }
    
    std::vector<std::string> params = {
        especialidadesJson,
        freelancer.getExperienciaAnos().has_value() ? std::to_string(*freelancer.getExperienciaAnos()) : "",
        freelancer.getPrecoMinimo().has_value() ? std::to_string(*freelancer.getPrecoMinimo()) : "",
        freelancer.getCpf().value_or(""),
        latStr,
        lonStr,
        std::to_string(freelancer.getRaioAtuacaoKm()),
        freelancer.getDisponibilidade().value_or(""),
        freelancer.isDocumentosVerificados() ? "true" : "false",
        freelancer.isAtivo() ? "true" : "false",
        freelancer.getId()
    };
    
    try {
        conn->executeParams(query, params);
        LOG_INFO("[FreelancerRepository] Freelancer updated successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("[FreelancerRepository] Error updating freelancer: " + std::string(e.what()));
        return false;
    }
}

bool FreelancerRepository::deleteById(const std::string& id) {
    LOG_INFO("[FreelancerRepository] deleteById() - id: " + id);
    
    auto conn = pool_->acquire();
    
    std::string query = "DELETE FROM freelancer_profiles WHERE id = $1";
    
    try {
        conn->executeParams(query, {id});
        LOG_INFO("[FreelancerRepository] Freelancer deleted successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("[FreelancerRepository] Error deleting freelancer: " + std::string(e.what()));
        return false;
    }
}

std::vector<FreelancerProfile> FreelancerRepository::findNearby(
    double latitude,
    double longitude,
    double radiusKm
) {
    LOG_DEBUG("[FreelancerRepository] findNearby() - lat: " + std::to_string(latitude) + 
              ", lng: " + std::to_string(longitude) + ", radius: " + std::to_string(radiusKm) + "km");
    
    auto conn = pool_->acquire();
    
    // Buscar freelancers dentro do raio de atuação deles
    std::string query = R"(
        SELECT *,
            (6371 * acos(
                cos(radians($1)) * cos(radians(latitude)) * 
                cos(radians(longitude) - radians($2)) + 
                sin(radians($1)) * sin(radians(latitude))
            )) AS distance
        FROM freelancer_profiles
        WHERE ativo = true 
            AND latitude IS NOT NULL 
            AND longitude IS NOT NULL
        HAVING distance <= raio_atuacao_km
        ORDER BY distance ASC
    )";
    
    std::vector<std::string> params = {
        std::to_string(latitude),
        std::to_string(longitude)
    };
    
    auto result = conn->executeParams(query, params);
    
    std::vector<FreelancerProfile> freelancers;
    for (int i = 0; i < result.rowCount(); i++) {
        freelancers.push_back(mapToFreelancer(result, i));
    }
    
    LOG_INFO("[FreelancerRepository] Found " + std::to_string(freelancers.size()) + " freelancers nearby");
    return freelancers;
}

} // namespace Domains::Freelances::Repositories

