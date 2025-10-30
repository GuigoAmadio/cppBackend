#include "OfferRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Offers::Repositories {

void OfferRepository::save(const Offer& offer) {
    LOG_DEBUG("[OfferRepository::save] id=" + offer.getId());
    auto conn = pool_->acquire();
    auto ins = conn->executeParams(
        "INSERT INTO offers (id, leilao_id, restaurante_id, freelancer_id, mensagem, shortlisted, valor_oferecido, valor_minimo_aceito) "
        "VALUES ($1, NULLIF($2,'')::uuid, $3, $4, $5, false, $6, $7)",
        { offer.getId(), offer.getLeilaoId().has_value()? offer.getLeilaoId().value() : "", offer.getRestauranteId(), offer.getUserId(), offer.getMensagem(), std::to_string(offer.getValorOferecido()), std::to_string(offer.getValorMinimoAceito()) }
    );
    if (!ins.isSuccess()) {
        LOG_ERROR(std::string("[OfferRepository::save] insert failed for id=") + offer.getId());
        LOG_ERROR(std::string("[OfferRepository::save] db error: ") + ins.getError());
        throw std::runtime_error("failed to insert offer");
    }
    // Verificação imediata
    try {
        auto chk = conn->executeParams("SELECT 1 FROM offers WHERE id::text = $1", { offer.getId() });
        LOG_DEBUG(std::string("[OfferRepository::save] verify id=") + offer.getId() + ", found=" + std::to_string(chk.rowCount()));
    } catch (const std::exception& e) {
        LOG_WARNING(std::string("[OfferRepository::save] verify failed: ") + e.what());
    }
    LOG_INFO("[OfferRepository::save] inserted id=" + offer.getId());
}

std::vector<OfferProfileProjection> OfferRepository::listForLeilaoAsProfiles(const std::string& leilaoId) {
    // Esta query assume existência de tabela users com nome/score/avaliacoes; adapte conforme seu schema
    LOG_DEBUG("[OfferRepository::listForLeilaoAsProfiles] leilao_id=" + leilaoId);
    auto res = pool_->acquire()->executeParams(
        "SELECT o.id, o.freelancer_id, coalesce(u.name,'?') as nome, coalesce(u.score,0) as score, 0 as num_aval, 0 as dist_km, o.shortlisted, to_char(o.submitted_at,'YYYY-MM-DD""T""HH24:MI:SS""Z""') FROM offers o LEFT JOIN users u ON u.id = o.freelancer_id WHERE o.leilao_id::text = $1",
        { leilaoId }
    );
    std::vector<OfferProfileProjection> v;
    if (!res.isSuccess()) return v;
    for (int r=0; r<res.rowCount(); ++r) {
        OfferProfileProjection p{};
        p.offerId = res.getValue(r,0);
        p.freelancerId = res.getValue(r,1);
        p.nome = res.getValue(r,2);
        p.score = std::stod(res.getValue(r,3));
        p.numAvaliacoes = std::stoi(res.getValue(r,4));
        p.distanciaKm = std::stod(res.getValue(r,5));
        p.shortlisted = (res.getValue(r,6) == "t" || res.getValue(r,6) == "true");
        p.createdAt = res.getValue(r,7);
        v.push_back(p);
    }
    LOG_INFO("[OfferRepository::listForLeilaoAsProfiles] count=" + std::to_string(v.size()));
    return v;
}

void OfferRepository::setShortlist(const std::string& offerId, bool shortlisted) {
    LOG_DEBUG("[OfferRepository::setShortlist] id=" + offerId + ", shortlisted=" + std::string(shortlisted?"true":"false"));
    pool_->acquire()->executeParams("UPDATE offers SET shortlisted = $2 WHERE id = $1", { offerId, shortlisted ? "true" : "false" });
}

std::optional<Offer> OfferRepository::findById(const std::string& id) {
    LOG_DEBUG("[OfferRepository::findById] id=" + id);
    auto res = pool_->acquire()->executeParams(
        "SELECT id, user_id, restaurante_id, leilao_id, mensagem, valor_oferecido, valor_minimo_aceito, to_char(submitted_at,'YYYY-MM-DD""T""HH24:MI:SS""Z""') FROM offers WHERE id::text = $1",
        { id }
    );
    if (!res.isSuccess() || res.rowCount() == 0) { LOG_WARNING("[OfferRepository::findById] not found id=" + id); return std::nullopt; }
    int r=0;
    std::optional<std::string> leilaoId = res.isNull(r,3) ? std::optional<std::string>{} : std::optional<std::string>{ res.getValue(r,3) };
    Offer o(
        res.getValue(r,0),
        res.getValue(r,1),
        res.getValue(r,2),
        leilaoId,
        std::stod(res.getValue(r,5)),
        std::stod(res.getValue(r,6)),
        res.getValue(r,4),
        res.getValue(r,7)
    );
    LOG_DEBUG("[OfferRepository::findById] found id=" + o.getId());
    return o;
}

void OfferRepository::accept(const std::string& id) {
    LOG_DEBUG("[OfferRepository::accept] id=" + id);
    pool_->acquire()->executeParams("UPDATE offers SET status='aceita', responded_at=NOW() WHERE id=$1", { id });
}

void OfferRepository::reject(const std::string& id) {
    LOG_DEBUG("[OfferRepository::reject] id=" + id);
    pool_->acquire()->executeParams("UPDATE offers SET status='rejeitada', responded_at=NOW() WHERE id=$1", { id });
}

std::string OfferRepository::generateId() {
    auto res = pool_->acquire()->execute("SELECT gen_random_uuid()::text");
    if (res.isSuccess() && res.rowCount() > 0) return res.getValue(0,0);
    return "generated-offer-id";
}

} // namespace Domains::Offers::Repositories


