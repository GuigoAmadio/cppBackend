#pragma once

#include <string>
#include <optional>
#include "../entities/Offer.hpp"
#include "../repositories/OfferRepository.hpp"
#include "../../leiloes/repositories/LeilaoRepository.hpp"

namespace Domains::Offers::UseCases {

using Domains::Offers::Entities::Offer;
using Domains::Offers::Repositories::OfferRepository;
using Domains::Leiloes::Repositories::LeilaoRepository;

struct CreateOfferInput {
    std::string userId;
    std::string restauranteId;
    std::optional<std::string> leilaoId; // null => nao_solicitada
    double valorOferecido;
    double valorMinimoAceito;
    std::string mensagem;
};

class CreateOfferUseCase {
public:
    CreateOfferUseCase(OfferRepository* offerRepo, LeilaoRepository* leilaoRepo)
        : offerRepo_(offerRepo), leilaoRepo_(leilaoRepo) {}

    Offer execute(const CreateOfferInput& input);

private:
    OfferRepository* offerRepo_;
    LeilaoRepository* leilaoRepo_;
};

} // namespace Domains::Offers::UseCases


