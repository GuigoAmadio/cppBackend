#include "CandidaturaRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>
#include <iomanip>
#include <random>

namespace Domains {
namespace Candidaturas {
namespace Repositories {

// Função auxiliar para gerar UUID
static std::string generateId() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    uint64_t part1 = dis(gen);
    uint64_t part2 = dis(gen);
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << (part1 >> 32);
    ss << "-" << std::setw(4) << ((part1 >> 16) & 0xFFFF);
    ss << "-4" << std::setw(3) << ((part1) & 0xFFF);
    ss << "-" << std::setw(4) << (0x8000 | ((part2 >> 48) & 0x3FFF));
    ss << "-" << std::setw(12) << (part2 & 0xFFFFFFFFFFFF);
    
    return ss.str();
}

CandidaturaRepository::CandidaturaRepository(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {}

std::string CandidaturaRepository::create(const Candidatura& candidatura) {
    LOG_INFO("[CandidaturaRepository] create() - vaga_id: " + candidatura.getVagaId());
    
    auto conn = pool_->acquire();
    std::string id = generateId();
    
    std::string query = R"(
        INSERT INTO candidaturas (
            id, vaga_id, freelancer_id, preco_proposto_freelancer, mensagem, status
        ) VALUES (
            $1::uuid, $2::uuid, $3::uuid, 
            CASE WHEN $4 = '' THEN NULL ELSE $4::decimal END,
            NULLIF($5,''), $6
        )
        RETURNING id
    )";
    
    std::vector<std::string> params = {
        id,
        candidatura.getVagaId(),
        candidatura.getFreelancerId(),
        candidatura.getPrecoPropostoFreelancer().has_value() 
            ? std::to_string(*candidatura.getPrecoPropostoFreelancer()) : "",
        candidatura.getMensagem().value_or(""),
        candidatura.getStatus()
    };
    
    // Log detalhado dos parâmetros
    LOG_DEBUG("[CandidaturaRepository] Params: id=" + params[0]);
    LOG_DEBUG("[CandidaturaRepository] Params: vaga_id=" + params[1]);
    LOG_DEBUG("[CandidaturaRepository] Params: freelancer_id=" + params[2]);
    LOG_DEBUG("[CandidaturaRepository] Params: preco_proposto=" + params[3]);
    LOG_DEBUG("[CandidaturaRepository] Params: mensagem=" + params[4]);
    LOG_DEBUG("[CandidaturaRepository] Params: status=" + params[5]);
    
    try {
        LOG_DEBUG("[CandidaturaRepository] Executing query...");
        auto result = conn->executeParams(query, params);
        LOG_DEBUG("[CandidaturaRepository] Query executed, checking result...");
        
        if (result.rowCount() == 0) {
            LOG_ERROR("[CandidaturaRepository] INSERT FAILED! No rows returned");
            throw std::runtime_error("INSERT into candidaturas failed - no rows returned");
        }
        
        std::string returnedId = result.getValue(0, 0);
        LOG_INFO("[CandidaturaRepository] Candidatura created successfully - ID: " + returnedId);
        return returnedId;
    } catch (const std::exception& e) {
        LOG_ERROR("[CandidaturaRepository] Error creating candidatura: " + std::string(e.what()));
        throw;
    }
}

std::optional<Candidatura> CandidaturaRepository::findById(const std::string& id) {
    LOG_DEBUG("[CandidaturaRepository] findById() - id: " + id);
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM candidaturas WHERE id = $1::uuid";
    auto result = conn->executeParams(query, {id});
    
    if (result.rowCount() == 0) {
        LOG_DEBUG("[CandidaturaRepository] Candidatura not found");
        return std::nullopt;
    }
    
    return mapToCandidatura(result, 0);
}

std::vector<Candidatura> CandidaturaRepository::findByVaga(const std::string& vagaId) {
    LOG_DEBUG("[CandidaturaRepository] findByVaga() - vaga_id: " + vagaId);
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM candidaturas WHERE vaga_id = $1::uuid ORDER BY created_at DESC";
    auto result = conn->executeParams(query, {vagaId});
    
    std::vector<Candidatura> candidaturas;
    for (int i = 0; i < result.rowCount(); i++) {
        candidaturas.push_back(mapToCandidatura(result, i));
    }
    
    LOG_INFO("[CandidaturaRepository] Found " + std::to_string(candidaturas.size()) + " candidaturas for vaga");
    return candidaturas;
}

std::vector<Candidatura> CandidaturaRepository::findByFreelancer(const std::string& freelancerId) {
    LOG_DEBUG("[CandidaturaRepository] findByFreelancer() - freelancer_id: " + freelancerId);
    auto conn = pool_->acquire();
    
    std::string query = "SELECT * FROM candidaturas WHERE freelancer_id = $1::uuid ORDER BY created_at DESC";
    auto result = conn->executeParams(query, {freelancerId});
    
    std::vector<Candidatura> candidaturas;
    for (int i = 0; i < result.rowCount(); i++) {
        candidaturas.push_back(mapToCandidatura(result, i));
    }
    
    LOG_INFO("[CandidaturaRepository] Found " + std::to_string(candidaturas.size()) + " candidaturas for freelancer");
    return candidaturas;
}

bool CandidaturaRepository::existsCandidatura(const std::string& vagaId, const std::string& freelancerId) {
    LOG_DEBUG("[CandidaturaRepository] existsCandidatura() - vaga: " + vagaId + ", freelancer: " + freelancerId);
    auto conn = pool_->acquire();
    
    std::string query = "SELECT COUNT(*) FROM candidaturas WHERE vaga_id = $1::uuid AND freelancer_id = $2::uuid";
    auto result = conn->executeParams(query, {vagaId, freelancerId});
    
    int count = std::stoi(result.getValue(0, 0));
    return count > 0;
}

void CandidaturaRepository::update(const Candidatura& candidatura) {
    LOG_INFO("[CandidaturaRepository] update() - id: " + candidatura.getId());
    auto conn = pool_->acquire();
    
    std::string query = R"(
        UPDATE candidaturas SET
            status = $1,
            motivo_rejeicao = NULLIF($2,''),
            responded_at = CASE WHEN $3 = '' THEN NULL ELSE $3::timestamp END
        WHERE id = $4::uuid
    )";
    
    std::vector<std::string> params = {
        candidatura.getStatus(),
        candidatura.getMotivoRejeicao().value_or(""),
        candidatura.getRespondedAt().value_or(""),
        candidatura.getId()
    };
    
    try {
        conn->executeParams(query, params);
        LOG_INFO("[CandidaturaRepository] Candidatura updated successfully");
    } catch (const std::exception& e) {
        LOG_ERROR("[CandidaturaRepository] Error updating candidatura: " + std::string(e.what()));
        throw;
    }
}

int CandidaturaRepository::countPendentesByVaga(const std::string& vagaId) {
    LOG_DEBUG("[CandidaturaRepository] countPendentesByVaga() - vaga_id: " + vagaId);
    auto conn = pool_->acquire();
    
    std::string query = "SELECT COUNT(*) FROM candidaturas WHERE vaga_id = $1::uuid AND status = 'pendente'";
    auto result = conn->executeParams(query, {vagaId});
    
    return std::stoi(result.getValue(0, 0));
}

Candidatura CandidaturaRepository::mapToCandidatura(const Core::Database::QueryResult& result, int row) {
    std::string id = result.getValue(row, 0);
    std::string vagaId = result.getValue(row, 1);
    std::string freelancerId = result.getValue(row, 2);
    
    std::optional<double> precoPropostoFreelancer;
    std::string precoStr = result.getValue(row, 3);
    if (!precoStr.empty()) {
        precoPropostoFreelancer = std::stod(precoStr);
    }
    
    std::optional<std::string> mensagem;
    std::string mensagemStr = result.getValue(row, 4);
    if (!mensagemStr.empty()) {
        mensagem = mensagemStr;
    }
    
    std::string status = result.getValue(row, 5);
    
    std::optional<std::string> motivoRejeicao;
    std::string motivoStr = result.getValue(row, 6);
    if (!motivoStr.empty()) {
        motivoRejeicao = motivoStr;
    }
    
    std::string createdAt = result.getValue(row, 7);
    
    std::optional<std::string> respondedAt;
    std::string respondedAtStr = result.getValue(row, 8);
    if (!respondedAtStr.empty()) {
        respondedAt = respondedAtStr;
    }
    
    return Candidatura(
        id, vagaId, freelancerId, precoPropostoFreelancer,
        mensagem, status, motivoRejeicao, createdAt, respondedAt
    );
}

} // namespace Repositories
} // namespace Candidaturas
} // namespace Domains

