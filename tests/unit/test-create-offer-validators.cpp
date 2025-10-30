#include <iostream>
#include <stdexcept>
#include "../../src/domains/offers/use_cases/CreateOfferUseCase.hpp"

using Domains::Offers::UseCases::CreateOfferUseCase;
using Domains::Offers::UseCases::CreateOfferInput;
using Domains::Offers::Repositories::OfferRepository;
using Domains::Leiloes::Repositories::LeilaoRepository;

int main() {
    // Não passaremos repositórios reais porque testaremos apenas validação que ocorre antes de tocar o repo
    CreateOfferUseCase uc((OfferRepository*)nullptr, (LeilaoRepository*)nullptr);

    bool threw = false;
    try {
        CreateOfferInput bad{ "user", "rest", std::optional<std::string>{}, /*oferecido*/ 100.0, /*minimo*/ 120.0, "msg" };
        uc.execute(bad);
    } catch (const std::exception&) { threw = true; }
    if (!threw) {
        std::cerr << "FAIL: expected exception when valor_minimo_aceito > valor_oferecido" << std::endl;
        return 1;
    }

    std::cout << "OK - test-create-offer-validators" << std::endl;
    return 0;
}


