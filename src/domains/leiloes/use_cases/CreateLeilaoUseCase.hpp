#pragma once

#include <string>
#include <optional>
#include "../entities/Leilao.hpp"
#include "../repositories/LeilaoRepository.hpp"

namespace Domains::Leiloes::UseCases {

using Domains::Leiloes::Entities::Leilao;
using Domains::Leiloes::Repositories::LeilaoRepository;

struct CreateLeilaoInput {
    std::string restauranteId;
    std::string titulo;
    std::string categoria;
    double valorIdeal;
    double valorMaxQuero;
    std::string dataTrabalho;       // ISO 8601
    std::optional<double> duracaoHoras;
    std::string dataLimiteOffers;   // ISO 8601
};

class CreateLeilaoUseCase {
public:
    explicit CreateLeilaoUseCase(LeilaoRepository* repo) : repo_(repo) {}

    Leilao execute(const CreateLeilaoInput& input);

private:
    LeilaoRepository* repo_;
};

} // namespace Domains::Leiloes::UseCases


