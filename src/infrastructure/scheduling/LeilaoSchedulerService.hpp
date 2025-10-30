#pragma once

#include <string>
#include "../../domains/leiloes/repositories/LeilaoRepository.hpp"
#include "../../domains/offers/repositories/OfferRepository.hpp"
#include "../../domains/offers/use_cases/AcceptOfferUseCase.hpp"

namespace Infrastructure::Scheduling {

using Domains::Leiloes::Repositories::LeilaoRepository;
using Domains::Offers::Repositories::OfferRepository;
using Domains::Offers::UseCases::AcceptOfferUseCase;

class LeilaoSchedulerService {
public:
    LeilaoSchedulerService(LeilaoRepository* lRepo, OfferRepository* oRepo)
        : leilaoRepo_(lRepo), offerRepo_(oRepo) {}

    void runOnce() {
        const std::string now = "now"; // TODO: clock util
        auto expirados = leilaoRepo_->listExpiredPendingClose(now);
        for (const auto& leilao : expirados) {
            auto perfis = offerRepo_->listForLeilaoAsProfiles(leilao.getId());

            // Preferir shortlisted; se nenhum, considerar todos
            std::vector<Domains::Offers::Repositories::OfferProfileProjection> candidatos;
            for (auto& p : perfis) if (p.shortlisted) candidatos.push_back(p);
            if (candidatos.empty()) candidatos = std::move(perfis);
            if (candidatos.empty()) { leilaoRepo_->close(leilao.getId()); continue; }

            // Ranking: maior score, depois numAvaliacoes, depois menor distancia, depois mais antigo
            std::sort(candidatos.begin(), candidatos.end(), [](const auto& a, const auto& b){
                if (a.score != b.score) return a.score > b.score;
                if (a.numAvaliacoes != b.numAvaliacoes) return a.numAvaliacoes > b.numAvaliacoes;
                if (a.distanciaKm != b.distanciaKm) return a.distanciaKm < b.distanciaKm;
                return a.createdAt < b.createdAt; // mais antigo primeiro
            });

            // Selecionar top-1 e aceitar
            AcceptOfferUseCase acceptUC(leilaoRepo_, offerRepo_);
            try {
                acceptUC.execute(candidatos.front().offerId);
            } catch (...) {
                // continuar próximos; em produção, logar
            }

            // Fechar leilão
            leilaoRepo_->close(leilao.getId());
        }
    }

private:
    LeilaoRepository* leilaoRepo_;
    OfferRepository* offerRepo_;
};

} // namespace Infrastructure::Scheduling


