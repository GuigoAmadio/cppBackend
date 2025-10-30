#include "CreateLeilaoUseCase.hpp"
#include <stdexcept>
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Leiloes::UseCases {

Leilao CreateLeilaoUseCase::execute(const CreateLeilaoInput& input) {
    LOG_DEBUG("[CreateLeilaoUseCase] rest=" + input.restauranteId + ", titulo=" + input.titulo + ", valorIdeal=" + std::to_string(input.valorIdeal) + ", valorMax=" + std::to_string(input.valorMaxQuero));
    // Validações básicas
    if (input.valorIdeal > input.valorMaxQuero) {
        throw std::runtime_error("valorIdeal deve ser <= valorMaxQuero");
    }
    if (!(input.dataLimiteOffers < input.dataTrabalho)) {
        throw std::runtime_error("dataLimiteOffers deve ser anterior a dataTrabalho");
    }

    std::string id = repo_->generateId();

    Leilao leilao(
        id,
        input.restauranteId,
        input.titulo,
        input.categoria,
        input.valorIdeal,
        input.valorMaxQuero,
        input.dataTrabalho,
        input.duracaoHoras,
        input.dataLimiteOffers
    );

    LOG_DEBUG("[CreateLeilaoUseCase] saving id=" + id);
    repo_->save(leilao);
    LOG_INFO("[CreateLeilaoUseCase] created id=" + id);
    return leilao;
}

} // namespace Domains::Leiloes::UseCases


