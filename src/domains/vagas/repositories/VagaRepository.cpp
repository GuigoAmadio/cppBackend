#include "VagaRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>
#include <random>
#include <iomanip>

namespace Domains::Vagas::Repositories {

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

Vaga VagaRepository::mapToVaga(const QueryResult& result, int row) {
    std::string id = result.getValue(row, 0);
    std::string restauranteId = result.getValue(row, 1);
    std::string titulo = result.getValue(row, 2);
    std::string categoria = result.getValue(row, 4);
    double precoOferecido = std::stod(result.getValue(row, 5));
    std::string dataTrabalho = result.getValue(row, 6);
    
    Vaga vaga(id, restauranteId, titulo, categoria, precoOferecido, dataTrabalho);
    
    // Campos opcionais
    std::string descricao = result.getValue(row, 3);
    if (!descricao.empty()) vaga.setDescricao(descricao);
    
    std::string horarioInicio = result.getValue(row, 7);
    if (!horarioInicio.empty()) vaga.setHorarioInicio(horarioInicio);
    
    std::string horarioFim = result.getValue(row, 8);
    if (!horarioFim.empty()) vaga.setHorarioFim(horarioFim);
    
    std::string duracaoStr = result.getValue(row, 9);
    if (!duracaoStr.empty()) vaga.setDuracaoHoras(std::stod(duracaoStr));
    
    std::string dataLimite = result.getValue(row, 10);
    if (!dataLimite.empty()) vaga.setDataLimiteCandidatura(dataLimite);
    
    std::string requisitos = result.getValue(row, 11);
    if (!requisitos.empty()) vaga.setRequisitos(requisitos);
    
    std::string vagasDisp = result.getValue(row, 12);
    if (!vagasDisp.empty()) vaga.setVagasDisponiveis(std::stoi(vagasDisp));
    
    std::string vagasPreen = result.getValue(row, 13);
    if (!vagasPreen.empty()) vaga.setVagasPreenchidas(std::stoi(vagasPreen));
    
    std::string status = result.getValue(row, 14);
    if (!status.empty()) vaga.setStatus(status);
    
    vaga.setCreatedAt(result.getValue(row, 15));
    vaga.setUpdatedAt(result.getValue(row, 16));
    
    return vaga;
}

std::string VagaRepository::create(const Vaga& vaga) {
    LOG_INFO("[VagaRepository] create() - restaurante_id: " + vaga.getRestauranteId());
    
    auto conn = pool_->acquire();
    std::string id = generateId();
    
    std::string query = R"(
        INSERT INTO vagas (
            id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho,
            vagas_disponiveis, vagas_preenchidas, status
        ) VALUES (
            $1::uuid, $2::uuid, $3, NULLIF($4,''), $5, $6::decimal, $7::timestamp, 
            $8::int, $9::int, $10
        )
        RETURNING id
    )";
    
    std::vector<std::string> params = {
        id, vaga.getRestauranteId(), vaga.getTitulo(), vaga.getDescricao().value_or(""),
        vaga.getCategoria(), std::to_string(vaga.getPrecoOferecido()), vaga.getDataTrabalho(),
        std::to_string(vaga.getVagasDisponiveis()), std::to_string(vaga.getVagasPreenchidas()),
        vaga.getStatus()
    };
    
    // Log detalhado dos parâmetros
    LOG_DEBUG("[VagaRepository] Params: id=" + params[0]);
    LOG_DEBUG("[VagaRepository] Params: restaurante_id=" + params[1]);
    LOG_DEBUG("[VagaRepository] Params: titulo=" + params[2]);
    LOG_DEBUG("[VagaRepository] Params: descricao=" + params[3]);
    LOG_DEBUG("[VagaRepository] Params: categoria=" + params[4]);
    LOG_DEBUG("[VagaRepository] Params: preco=" + params[5]);
    LOG_DEBUG("[VagaRepository] Params: data_trabalho=" + params[6]);
    LOG_DEBUG("[VagaRepository] Params: vagas_disp=" + params[7]);
    LOG_DEBUG("[VagaRepository] Params: vagas_preen=" + params[8]);
    LOG_DEBUG("[VagaRepository] Params: status=" + params[9]);
    
    try {
        LOG_DEBUG("[VagaRepository] ========== FULL QUERY ==========");
        LOG_DEBUG(query);
        LOG_DEBUG("[VagaRepository] ========== PARAMS COUNT: " + std::to_string(params.size()) + " ==========");
        LOG_DEBUG("[VagaRepository] Executing query...");
        auto result = conn->executeParams(query, params);
        LOG_DEBUG("[VagaRepository] Query executed, checking result...");
        
        // Verificar se houve erro
        if (result.rowCount() == 0) {
            LOG_ERROR("[VagaRepository] INSERT FAILED! No rows returned from RETURNING clause");
            LOG_ERROR("[VagaRepository] This means the INSERT was rejected by PostgreSQL");
            // Tentar executar a query manualmente para ver o erro
            throw std::runtime_error("INSERT into vagas failed - no rows returned");
        }
        
        std::string returnedId = result.getValue(0, 0);
        LOG_INFO("[VagaRepository] Vaga created successfully - ID: " + returnedId);
        return returnedId;
    } catch (const std::exception& e) {
        LOG_ERROR("[VagaRepository] Error creating vaga: " + std::string(e.what()));
        throw;
    }
}

std::optional<Vaga> VagaRepository::findById(const std::string& id) {
    LOG_DEBUG("[VagaRepository] findById() - id: " + id);
    auto conn = pool_->acquire();
    
    // Test: verificar se a vaga existe de alguma forma
    auto testResult = conn->executeParams("SELECT COUNT(*) as total FROM vagas", {});
    LOG_DEBUG("[VagaRepository] Total vagas in DB: " + testResult.getValue(0, 0));
    
    auto result = conn->executeParams("SELECT * FROM vagas WHERE id = $1::uuid", {id});
    LOG_DEBUG("[VagaRepository] Query result: " + std::to_string(result.rowCount()) + " rows");
    
    if (result.rowCount() == 0) {
        // Debug: tentar buscar sem cast
        auto result2 = conn->executeParams("SELECT * FROM vagas WHERE CAST(id AS TEXT) = $1", {id});
        LOG_DEBUG("[VagaRepository] Query with CAST result: " + std::to_string(result2.rowCount()) + " rows");
    }
    
    return result.rowCount() > 0 ? std::optional<Vaga>(mapToVaga(result, 0)) : std::nullopt;
}

std::vector<Vaga> VagaRepository::findByRestaurante(const std::string& restauranteId) {
    LOG_DEBUG("[VagaRepository] findByRestaurante() - restaurante_id: " + restauranteId);
    auto conn = pool_->acquire();
    auto result = conn->executeParams("SELECT * FROM vagas WHERE restaurante_id = $1::uuid ORDER BY created_at DESC", {restauranteId});
    
    std::vector<Vaga> vagas;
    for (int i = 0; i < result.rowCount(); i++) {
        vagas.push_back(mapToVaga(result, i));
    }
    return vagas;
}

std::vector<Vaga> VagaRepository::findByCategoria(const std::string& categoria) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams("SELECT * FROM vagas WHERE categoria = $1 AND status = 'aberta' ORDER BY created_at DESC", {categoria});
    
    std::vector<Vaga> vagas;
    for (int i = 0; i < result.rowCount(); i++) {
        vagas.push_back(mapToVaga(result, i));
    }
    return vagas;
}

std::vector<Vaga> VagaRepository::findByStatus(const std::string& status) {
    auto conn = pool_->acquire();
    auto result = conn->executeParams("SELECT * FROM vagas WHERE status = $1 ORDER BY created_at DESC", {status});
    
    std::vector<Vaga> vagas;
    for (int i = 0; i < result.rowCount(); i++) {
        vagas.push_back(mapToVaga(result, i));
    }
    return vagas;
}

std::vector<Vaga> VagaRepository::findAbertas() {
    return findByStatus("aberta");
}

std::vector<Vaga> VagaRepository::findAll() {
    auto conn = pool_->acquire();
    auto result = conn->executeParams("SELECT * FROM vagas ORDER BY created_at DESC", {});
    
    std::vector<Vaga> vagas;
    for (int i = 0; i < result.rowCount(); i++) {
        vagas.push_back(mapToVaga(result, i));
    }
    return vagas;
}

bool VagaRepository::update(const Vaga& vaga) {
    LOG_INFO("[VagaRepository] update() - id: " + vaga.getId());
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE vagas SET
            titulo = $1, descricao = NULLIF($2,''), categoria = $3, preco_oferecido = $4,
            data_trabalho = $5, horario_inicio = NULLIF($6,''), horario_fim = NULLIF($7,''),
            duracao_horas = NULLIF($8,'')::decimal, data_limite_candidatura = NULLIF($9,'')::timestamp,
            requisitos = NULLIF($10,'')::jsonb, vagas_disponiveis = $11, vagas_preenchidas = $12,
            status = $13, updated_at = CURRENT_TIMESTAMP
        WHERE id = $14
    )";
    
    std::vector<std::string> params = {
        vaga.getTitulo(), vaga.getDescricao().value_or(""), vaga.getCategoria(),
        std::to_string(vaga.getPrecoOferecido()), vaga.getDataTrabalho(),
        vaga.getHorarioInicio().value_or(""), vaga.getHorarioFim().value_or(""),
        vaga.getDuracaoHoras().has_value() ? std::to_string(*vaga.getDuracaoHoras()) : "",
        vaga.getDataLimiteCandidatura().value_or(""), vaga.getRequisitos().value_or(""),
        std::to_string(vaga.getVagasDisponiveis()), std::to_string(vaga.getVagasPreenchidas()),
        vaga.getStatus(), vaga.getId()
    };
    
    conn->executeParams(query, params);
    return true;
}

bool VagaRepository::deleteById(const std::string& id) {
    auto conn = pool_->acquire();
    conn->executeParams("DELETE FROM vagas WHERE id = $1", {id});
    return true;
}

} // namespace Domains::Vagas::Repositories

