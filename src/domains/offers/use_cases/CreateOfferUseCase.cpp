#include "CreateOfferUseCase.hpp"
#include <stdexcept>
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Offers::UseCases {

Offer CreateOfferUseCase::execute(const CreateOfferInput& input) {
    LOG_DEBUG("[CreateOfferUseCase] user=" + input.userId + ", rest=" + input.restauranteId + (input.leilaoId.has_value()? ", leilao=" + input.leilaoId.value() : ", leilao=null") + ", oferecido=" + std::to_string(input.valorOferecido) + ", minimo=" + std::to_string(input.valorMinimoAceito));
    if (input.valorMinimoAceito > input.valorOferecido) {
        throw std::runtime_error("valorMinimoAceito deve ser <= valorOferecido");
    }

    // Se for em leilao, validar que leilao está aberto
    if (input.leilaoId.has_value()) {
        auto l = leilaoRepo_->findById(input.leilaoId.value());
        if (!l.has_value()) {
            throw std::runtime_error("Leilao não encontrado");
        }
        // Aqui poderíamos checar data atual; placeholder
    }

    std::string id = offerRepo_->generateId();
    std::string now = "now"; // placeholder

    Offer offer(
        id,
        input.userId,
        input.restauranteId,
        input.leilaoId,
        input.valorOferecido,
        input.valorMinimoAceito,
        input.mensagem,
        now
    );

    LOG_DEBUG("[CreateOfferUseCase] saving offer id=" + id);
    offerRepo_->save(offer);
    LOG_INFO("[CreateOfferUseCase] created id=" + id);
    return offer;
}

} // namespace Domains::Offers::UseCases


