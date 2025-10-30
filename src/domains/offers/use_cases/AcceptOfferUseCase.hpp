#pragma once

#include <string>
#include <optional>
#include "../../leiloes/repositories/LeilaoRepository.hpp"
#include "../repositories/OfferRepository.hpp"

namespace Domains::Offers::UseCases {

using Domains::Leiloes::Repositories::LeilaoRepository;
using Domains::Offers::Repositories::OfferRepository;

struct AcceptOfferResult {
    std::string jobId;
    std::string freelancerId;
    double valorFinal;
};

class AcceptOfferUseCase {
public:
    AcceptOfferUseCase(LeilaoRepository* leilaoRepo, OfferRepository* offerRepo)
        : leilaoRepo_(leilaoRepo), offerRepo_(offerRepo) {}

    AcceptOfferResult execute(const std::string& offerId);

    static double clampPreco(double oferecido, double minimoAceito, double maxRestaurante) {
        if (oferecido < minimoAceito) return minimoAceito;
        if (oferecido > maxRestaurante) return maxRestaurante;
        return oferecido;
    }

private:
    LeilaoRepository* leilaoRepo_;
    OfferRepository* offerRepo_;
};

} // namespace Domains::Offers::UseCases


