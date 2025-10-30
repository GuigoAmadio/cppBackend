#include "LeilaoRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Leiloes::Repositories {

std::optional<Leilao> LeilaoRepository::findById(const std::string& id) {
    LOG_DEBUG("[LeilaoRepository::findById] id=" + id);
    auto conn = pool_->acquire();
    auto res = conn->executeParams(
        "SELECT id, restaurante_id, titulo, categoria, valor_ideal, valor_max_quero, to_char(data_trabalho,'YYYY-MM-DD""T""HH24:MI:SS""Z""'), duracao_horas, to_char(data_limite_offers,'YYYY-MM-DD""T""HH24:MI:SS""Z""'), vagas_disponiveis, status FROM leiloes WHERE id::text = $1",
        { id }
    );
    if (!res.isSuccess() || res.rowCount() == 0) {
        LOG_WARNING("[LeilaoRepository::findById] not found id=" + id);
        try {
            auto conn2 = pool_->acquire();
            auto cnt = conn2->execute("SELECT count(*) FROM leiloes");
            std::string total = cnt.isSuccess() && cnt.rowCount() > 0 ? cnt.getValue(0,0) : "?";
            auto sample = conn2->execute("SELECT id::text FROM leiloes LIMIT 3");
            std::string s;
            for (int r=0; r<sample.rowCount(); ++r) { if (!s.empty()) s += ","; s += sample.getValue(r,0); }
            LOG_DEBUG("[LeilaoRepository::findById] table_size=" + total + ", sample_ids=[" + s + "]");
        } catch (...) {}
        return std::nullopt;
    }
    int r = 0;
    Leilao l(
        res.getValue(r,0), res.getValue(r,1), res.getValue(r,2), res.getValue(r,3),
        std::stod(res.getValue(r,4)), std::stod(res.getValue(r,5)),
        res.getValue(r,6),
        res.isNull(r,7) ? std::optional<double>{} : std::optional<double>{ std::stod(res.getValue(r,7)) },
        res.getValue(r,8)
    );
    LOG_DEBUG("[LeilaoRepository::findById] found id=" + l.getId());
    return l;
}

std::vector<Leilao> LeilaoRepository::listOpen(const std::string& nowIso8601) {
    LOG_DEBUG("[LeilaoRepository::listOpen] now=" + nowIso8601);
    auto conn = pool_->acquire();
    auto res = conn->executeParams(
        "SELECT id, restaurante_id, titulo, categoria, valor_ideal, valor_max_quero, to_char(data_trabalho,'YYYY-MM-DD""T""HH24:MI:SS""Z""'), duracao_horas, to_char(data_limite_offers,'YYYY-MM-DD""T""HH24:MI:SS""Z""') FROM leiloes WHERE status='aberto' AND data_limite_offers > $1",
        { nowIso8601 }
    );
    std::vector<Leilao> v;
    if (!res.isSuccess()) return v;
    for (int r=0; r<res.rowCount(); ++r) {
        v.emplace_back(
            res.getValue(r,0), res.getValue(r,1), res.getValue(r,2), res.getValue(r,3),
            std::stod(res.getValue(r,4)), std::stod(res.getValue(r,5)),
            res.getValue(r,6),
            res.isNull(r,7) ? std::optional<double>{} : std::optional<double>{ std::stod(res.getValue(r,7)) },
            res.getValue(r,8)
        );
    }
    LOG_INFO("[LeilaoRepository::listOpen] count=" + std::to_string(v.size()));
    return v;
}

std::vector<Leilao> LeilaoRepository::listExpiredPendingClose(const std::string& nowIso8601) {
    LOG_DEBUG("[LeilaoRepository::listExpiredPendingClose] now=" + nowIso8601);
    auto conn = pool_->acquire();
    auto res = conn->executeParams(
        "SELECT id, restaurante_id, titulo, categoria, valor_ideal, valor_max_quero, to_char(data_trabalho,'YYYY-MM-DD""T""HH24:MI:SS""Z""'), duracao_horas, to_char(data_limite_offers,'YYYY-MM-DD""T""HH24:MI:SS""Z""') FROM leiloes WHERE status='aberto' AND data_limite_offers <= $1",
        { nowIso8601 }
    );
    std::vector<Leilao> v;
    if (!res.isSuccess()) return v;
    for (int r=0; r<res.rowCount(); ++r) {
        v.emplace_back(
            res.getValue(r,0), res.getValue(r,1), res.getValue(r,2), res.getValue(r,3),
            std::stod(res.getValue(r,4)), std::stod(res.getValue(r,5)),
            res.getValue(r,6),
            res.isNull(r,7) ? std::optional<double>{} : std::optional<double>{ std::stod(res.getValue(r,7)) },
            res.getValue(r,8)
        );
    }
    LOG_INFO("[LeilaoRepository::listExpiredPendingClose] count=" + std::to_string(v.size()));
    return v;
}

void LeilaoRepository::save(const Leilao& leilao) {
    LOG_DEBUG("[LeilaoRepository::save] id=" + leilao.getId());
    auto conn = pool_->acquire();
    // Tratar duracao_horas opcional usando NULLIF
    std::string duracaoParam = leilao.getDuracaoHoras().has_value() ? std::to_string(leilao.getDuracaoHoras().value()) : "";
    auto ins = conn->executeParams(
        "INSERT INTO leiloes (id, restaurante_id, titulo, categoria, valor_ideal, valor_max_quero, data_trabalho, duracao_horas, data_limite_offers) "
        "VALUES ($1,$2,$3,$4,$5,$6,$7, NULLIF($8,'')::numeric, $9)",
        { leilao.getId(), leilao.getRestauranteId(), leilao.getTitulo(), leilao.getCategoria(),
          std::to_string(leilao.getValorIdeal()), std::to_string(leilao.getValorMaxQuero()),
          leilao.getDataTrabalho(), duracaoParam, leilao.getDataLimiteOffers() }
    );
    if (!ins.isSuccess()) {
        LOG_ERROR(std::string("[LeilaoRepository::save] insert failed for id=") + leilao.getId());
        LOG_ERROR(std::string("[LeilaoRepository::save] db error: ") + ins.getError());
        throw std::runtime_error("failed to insert leilao");
    }
    LOG_INFO("[LeilaoRepository::save] inserted id=" + leilao.getId());
    // Verificação imediata
    try {
        auto chk = conn->executeParams("SELECT 1 FROM leiloes WHERE id::text = $1", { leilao.getId() });
        LOG_DEBUG(std::string("[LeilaoRepository::save] verify id=") + leilao.getId() + ", found=" + std::to_string(chk.rowCount()));
    } catch (const std::exception& e) {
        LOG_WARNING(std::string("[LeilaoRepository::save] verify failed: ") + e.what());
    }
}

void LeilaoRepository::close(const std::string& id) {
    LOG_DEBUG("[LeilaoRepository::close] id=" + id);
    auto conn = pool_->acquire();
    conn->executeParams("UPDATE leiloes SET status='fechado' WHERE id=$1", { id });
}

std::string LeilaoRepository::generateId() {
    auto conn = pool_->acquire();
    auto res = conn->execute("SELECT gen_random_uuid()::text");
    if (res.isSuccess() && res.rowCount() > 0) {
        return res.getValue(0,0);
    }
    // fallback simples
    return "generated-leilao-id";
}

} // namespace Domains::Leiloes::Repositories


