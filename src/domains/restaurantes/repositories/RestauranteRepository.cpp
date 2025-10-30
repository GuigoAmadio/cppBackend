#include "RestauranteRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>
#include <random>
#include <iomanip>

namespace Domains::Restaurantes::Repositories {

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

RestauranteProfile RestauranteRepository::mapToRestaurante(
    const QueryResult& result,
    int row
) {
    LOG_DEBUG("[RestauranteRepository] Mapping row " + std::to_string(row) + " to RestauranteProfile");
    
    std::string id = result.getValue(row, 0);
    std::string userId = result.getValue(row, 1);
    std::string cnpj = result.getValue(row, 2);
    std::string razaoSocial = result.getValue(row, 3);
    double latitude = std::stod(result.getValue(row, 8));  // FIX: col 8
    double longitude = std::stod(result.getValue(row, 9)); // FIX: col 9
    
    RestauranteProfile restaurante(id, userId, cnpj, razaoSocial, latitude, longitude);
    
    // nome_fantasia (col 4)
    std::string nomeFantasia = result.getValue(row, 4);
    if (!nomeFantasia.empty()) {
        restaurante.setNomeFantasia(nomeFantasia);
    }
    
    // descricao (col 5)
    std::string descricao = result.getValue(row, 5);
    if (!descricao.empty()) {
        restaurante.setDescricao(descricao);
    }
    
    // categoria (col 6)
    std::string categoria = result.getValue(row, 6);
    if (!categoria.empty()) {
        restaurante.setCategoria(categoria);
    }
    
    // logo (col 7)
    std::string logo = result.getValue(row, 7);
    if (!logo.empty()) {
        restaurante.setLogo(logo);
    }
    
    // endereco_completo (col 10)
    std::string endereco = result.getValue(row, 10);
    if (!endereco.empty()) {
        restaurante.setEnderecoCompleto(endereco);
    }
    
    // cep (col 11)
    std::string cep = result.getValue(row, 11);
    if (!cep.empty()) {
        restaurante.setCep(cep);
    }
    
    // cidade (col 12)
    std::string cidade = result.getValue(row, 12);
    if (!cidade.empty()) {
        restaurante.setCidade(cidade);
    }
    
    // estado (col 13)
    std::string estado = result.getValue(row, 13);
    if (!estado.empty()) {
        restaurante.setEstado(estado);
    }
    
    // horario_funcionamento (col 14)
    std::string horario = result.getValue(row, 14);
    if (!horario.empty()) {
        restaurante.setHorarioFuncionamento(horario);
    }
    
    // documentos_verificados (col 15)
    std::string docVerif = result.getValue(row, 15);
    if (!docVerif.empty()) {
        restaurante.setDocumentosVerificados(docVerif == "t" || docVerif == "true" || docVerif == "1");
    }
    
    // ativo (col 16)
    std::string ativo = result.getValue(row, 16);
    if (!ativo.empty()) {
        restaurante.setAtivo(ativo == "t" || ativo == "true" || ativo == "1");
    }
    
    // created_at (col 17)
    std::string createdAt = result.getValue(row, 17);
    if (!createdAt.empty()) {
        restaurante.setCreatedAt(createdAt);
    }
    
    // updated_at (col 18)
    std::string updatedAt = result.getValue(row, 18);
    if (!updatedAt.empty()) {
        restaurante.setUpdatedAt(updatedAt);
    }
    
    return restaurante;
}

std::string RestauranteRepository::create(const RestauranteProfile& restaurante) {
    LOG_INFO("[RestauranteRepository] create() - user_id: " + restaurante.getUserId());
    LOG_DEBUG("[RestauranteRepository] cnpj: " + restaurante.getCnpj());
    LOG_DEBUG("[RestauranteRepository] razao_social: " + restaurante.getRazaoSocial());
    
    auto conn = pool_->acquire();
    LOG_DEBUG("[RestauranteRepository] Connection acquired");
    
    std::string id = generateId();
    LOG_DEBUG("[RestauranteRepository] Generated ID: " + id);
    
    std::string query = R"(
        INSERT INTO restaurante_profiles (
            id, user_id, cnpj, razao_social, nome_fantasia, descricao, categoria, logo,
            latitude, longitude, endereco_completo, cep, cidade, estado,
            horario_funcionamento, documentos_verificados, ativo
        ) VALUES (
            $1, $2, $3, $4, NULLIF($5,''), NULLIF($6,''), NULLIF($7,''), NULLIF($8,''),
            $9, $10, NULLIF($11,''), NULLIF($12,''), NULLIF($13,''), NULLIF($14,''),
            NULLIF($15,'')::jsonb, $16, $17
        )
        RETURNING id
    )";
    
    std::vector<std::string> params = {
        id,
        restaurante.getUserId(),
        restaurante.getCnpj(),
        restaurante.getRazaoSocial(),
        restaurante.getNomeFantasia().value_or(""),
        restaurante.getDescricao().value_or(""),
        restaurante.getCategoria().value_or(""),
        restaurante.getLogo().value_or(""),
        std::to_string(restaurante.getCoordenadas().getLatitude()),
        std::to_string(restaurante.getCoordenadas().getLongitude()),
        restaurante.getEnderecoCompleto().value_or(""),
        restaurante.getCep().value_or(""),
        restaurante.getCidade().value_or(""),
        restaurante.getEstado().value_or(""),
        restaurante.getHorarioFuncionamento().value_or(""),
        restaurante.isDocumentosVerificados() ? "true" : "false",
        restaurante.isAtivo() ? "true" : "false"
    };
    
    try {
        auto result = conn->executeParams(query, params);
        
        // Verificar se o INSERT foi bem-sucedido
        if (result.rowCount() == 0) {
            LOG_ERROR("[RestauranteRepository] INSERT FAILED! No rows returned from RETURNING clause");
            throw std::runtime_error("INSERT into restaurante_profiles failed - no rows returned");
        }
        
        std::string returnedId = result.getValue(0, 0);
        LOG_INFO("[RestauranteRepository] Restaurante created successfully - ID: " + returnedId);
        return returnedId;
    } catch (const std::exception& e) {
        LOG_ERROR("[RestauranteRepository] Error creating restaurante: " + std::string(e.what()));
        throw;
    }
}

std::optional<RestauranteProfile> RestauranteRepository::findById(const std::string& id) {
    LOG_DEBUG("[RestauranteRepository] findById() - id: " + id);
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM restaurante_profiles WHERE id = $1";
    auto result = conn->executeParams(query, {id});
    
    if (result.rowCount() == 0) {
        LOG_DEBUG("[RestauranteRepository] Restaurante not found");
        return std::nullopt;
    }
    
    LOG_INFO("[RestauranteRepository] Restaurante found - ID: " + id);
    return mapToRestaurante(result, 0);
}

std::optional<RestauranteProfile> RestauranteRepository::findByUserId(const std::string& userId) {
    LOG_DEBUG("[RestauranteRepository] findByUserId() - user_id: " + userId);
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM restaurante_profiles WHERE user_id = $1";
    auto result = conn->executeParams(query, {userId});
    
    if (result.rowCount() == 0) {
        LOG_DEBUG("[RestauranteRepository] Restaurante not found for user");
        return std::nullopt;
    }
    
    LOG_INFO("[RestauranteRepository] Restaurante found for user: " + userId);
    return mapToRestaurante(result, 0);
}

std::optional<RestauranteProfile> RestauranteRepository::findByCnpj(const std::string& cnpj) {
    LOG_DEBUG("[RestauranteRepository] findByCnpj() - cnpj: " + cnpj);
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM restaurante_profiles WHERE cnpj = $1";
    auto result = conn->executeParams(query, {cnpj});
    
    if (result.rowCount() == 0) {
        LOG_DEBUG("[RestauranteRepository] Restaurante not found for CNPJ");
        return std::nullopt;
    }
    
    LOG_INFO("[RestauranteRepository] Restaurante found for CNPJ: " + cnpj);
    return mapToRestaurante(result, 0);
}

std::vector<RestauranteProfile> RestauranteRepository::findAll() {
    LOG_DEBUG("[RestauranteRepository] findAll()");
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM restaurante_profiles ORDER BY created_at DESC";
    auto result = conn->executeParams(query, {});
    
    std::vector<RestauranteProfile> restaurantes;
    for (int i = 0; i < result.rowCount(); i++) {
        restaurantes.push_back(mapToRestaurante(result, i));
    }
    
    LOG_INFO("[RestauranteRepository] Found " + std::to_string(restaurantes.size()) + " restaurantes");
    return restaurantes;
}

std::vector<RestauranteProfile> RestauranteRepository::findByCategoria(const std::string& categoria) {
    LOG_DEBUG("[RestauranteRepository] findByCategoria() - categoria: " + categoria);
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM restaurante_profiles WHERE categoria = $1 ORDER BY created_at DESC";
    auto result = conn->executeParams(query, {categoria});
    
    std::vector<RestauranteProfile> restaurantes;
    for (int i = 0; i < result.rowCount(); i++) {
        restaurantes.push_back(mapToRestaurante(result, i));
    }
    
    LOG_INFO("[RestauranteRepository] Found " + std::to_string(restaurantes.size()) + " restaurantes for categoria");
    return restaurantes;
}

std::vector<RestauranteProfile> RestauranteRepository::findByCidade(const std::string& cidade) {
    LOG_DEBUG("[RestauranteRepository] findByCidade() - cidade: " + cidade);
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM restaurante_profiles WHERE cidade = $1 ORDER BY created_at DESC";
    auto result = conn->executeParams(query, {cidade});
    
    std::vector<RestauranteProfile> restaurantes;
    for (int i = 0; i < result.rowCount(); i++) {
        restaurantes.push_back(mapToRestaurante(result, i));
    }
    
    LOG_INFO("[RestauranteRepository] Found " + std::to_string(restaurantes.size()) + " restaurantes for cidade");
    return restaurantes;
}

std::vector<RestauranteProfile> RestauranteRepository::findAtivos() {
    LOG_DEBUG("[RestauranteRepository] findAtivos()");
    
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM restaurante_profiles WHERE ativo = true ORDER BY created_at DESC";
    auto result = conn->executeParams(query, {});
    
    std::vector<RestauranteProfile> restaurantes;
    for (int i = 0; i < result.rowCount(); i++) {
        restaurantes.push_back(mapToRestaurante(result, i));
    }
    
    LOG_INFO("[RestauranteRepository] Found " + std::to_string(restaurantes.size()) + " restaurantes ativos");
    return restaurantes;
}

bool RestauranteRepository::update(const RestauranteProfile& restaurante) {
    LOG_INFO("[RestauranteRepository] update() - id: " + restaurante.getId());
    
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE restaurante_profiles SET
            razao_social = $1,
            nome_fantasia = NULLIF($2,''),
            descricao = NULLIF($3,''),
            categoria = NULLIF($4,''),
            logo = NULLIF($5,''),
            latitude = $6,
            longitude = $7,
            endereco_completo = NULLIF($8,''),
            cep = NULLIF($9,''),
            cidade = NULLIF($10,''),
            estado = NULLIF($11,''),
            horario_funcionamento = NULLIF($12,'')::jsonb,
            documentos_verificados = $13,
            ativo = $14,
            updated_at = CURRENT_TIMESTAMP
        WHERE id = $15
    )";
    
    std::vector<std::string> params = {
        restaurante.getRazaoSocial(),
        restaurante.getNomeFantasia().value_or(""),
        restaurante.getDescricao().value_or(""),
        restaurante.getCategoria().value_or(""),
        restaurante.getLogo().value_or(""),
        std::to_string(restaurante.getCoordenadas().getLatitude()),
        std::to_string(restaurante.getCoordenadas().getLongitude()),
        restaurante.getEnderecoCompleto().value_or(""),
        restaurante.getCep().value_or(""),
        restaurante.getCidade().value_or(""),
        restaurante.getEstado().value_or(""),
        restaurante.getHorarioFuncionamento().value_or(""),
        restaurante.isDocumentosVerificados() ? "true" : "false",
        restaurante.isAtivo() ? "true" : "false",
        restaurante.getId()
    };
    
    try {
        conn->executeParams(query, params);
        LOG_INFO("[RestauranteRepository] Restaurante updated successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("[RestauranteRepository] Error updating restaurante: " + std::string(e.what()));
        return false;
    }
}

bool RestauranteRepository::deleteById(const std::string& id) {
    LOG_INFO("[RestauranteRepository] deleteById() - id: " + id);
    
    auto conn = pool_->acquire();
    
    std::string query = "DELETE FROM restaurante_profiles WHERE id = $1";
    
    try {
        conn->executeParams(query, {id});
        LOG_INFO("[RestauranteRepository] Restaurante deleted successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("[RestauranteRepository] Error deleting restaurante: " + std::string(e.what()));
        return false;
    }
}

std::vector<RestauranteProfile> RestauranteRepository::findNearby(
    double latitude,
    double longitude,
    double radiusKm
) {
    LOG_DEBUG("[RestauranteRepository] findNearby() - lat: " + std::to_string(latitude) + 
              ", lng: " + std::to_string(longitude) + ", radius: " + std::to_string(radiusKm) + "km");
    
    auto conn = pool_->acquire();
    
    // Usa fórmula de Haversine para calcular distância
    std::string query = R"(
        SELECT *,
            (6371 * acos(
                cos(radians($1)) * cos(radians(latitude)) * 
                cos(radians(longitude) - radians($2)) + 
                sin(radians($1)) * sin(radians(latitude))
            )) AS distance
        FROM restaurante_profiles
        WHERE ativo = true
        HAVING distance < $3
        ORDER BY distance ASC
    )";
    
    std::vector<std::string> params = {
        std::to_string(latitude),
        std::to_string(longitude),
        std::to_string(radiusKm)
    };
    
    auto result = conn->executeParams(query, params);
    
    std::vector<RestauranteProfile> restaurantes;
    for (int i = 0; i < result.rowCount(); i++) {
        restaurantes.push_back(mapToRestaurante(result, i));
    }
    
    LOG_INFO("[RestauranteRepository] Found " + std::to_string(restaurantes.size()) + " restaurantes nearby");
    return restaurantes;
}

} // namespace Domains::Restaurantes::Repositories

