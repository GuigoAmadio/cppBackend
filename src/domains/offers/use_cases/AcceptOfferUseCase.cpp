#include "AcceptOfferUseCase.hpp"
#include <stdexcept>
#include "../repositories/OfferRepository.hpp"
#include "../../leiloes/repositories/LeilaoRepository.hpp"

namespace Domains::Offers::UseCases {

AcceptOfferResult AcceptOfferUseCase::execute(const std::string& offerId) {
    auto offerOpt = offerRepo_->findById(offerId);
    if (!offerOpt.has_value()) {
        throw std::runtime_error("Offer não encontrada");
    }
    auto offer = offerOpt.value();

    double maxRestaurante = 1e9;
    if (offer.getLeilaoId().has_value()) {
        auto leilaoOpt = leilaoRepo_->findById(offer.getLeilaoId().value());
        if (!leilaoOpt.has_value()) {
            throw std::runtime_error("Leilao não encontrado para a offer");
        }
        maxRestaurante = leilaoOpt->getValorMaxQuero();
    }

    const double valorFinal = clampPreco(
        offer.getValorOferecido(),
        offer.getValorMinimoAceito(),
        maxRestaurante
    );

    offerRepo_->accept(offerId);

    AcceptOfferResult res{ /*jobId*/"job-generated-id", offer.getUserId(), valorFinal };
    return res;
}

} // namespace Domains::Offers::UseCases


