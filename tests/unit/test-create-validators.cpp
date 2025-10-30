#include <iostream>
#include <stdexcept>
#include "../../src/domains/leiloes/use_cases/CreateLeilaoUseCase.hpp"

using Domains::Leiloes::UseCases::CreateLeilaoUseCase;
using Domains::Leiloes::UseCases::CreateLeilaoInput;

// Repo fake minimal que não toca DB, só cumpre a interface necessária
namespace Domains { namespace Leiloes { namespace Repositories {
class LeilaoRepository; // fwd
}}}

int main() {
    // Não instanciamos repo real; aqui só verificamos validações por exceção
    CreateLeilaoUseCase uc((Domains::Leiloes::Repositories::LeilaoRepository*)nullptr);

    bool threw = false;
    try {
        CreateLeilaoInput bad{ "rest", "t", "cat", 200, 120, "2025-12-20T10:00:00Z", 4.0, "2025-12-19T10:00:00Z" };
        uc.execute(bad);
    } catch (const std::exception&) { threw = true; }
    if (!threw) { std::cerr << "FAIL: expected exception for valorIdeal > valorMaxQuero" << std::endl; return 1; }

    threw = false;
    try {
        CreateLeilaoInput bad2{ "rest", "t", "cat", 120, 180, "2025-12-19T10:00:00Z", 4.0, "2025-12-20T10:00:00Z" };
        uc.execute(bad2);
    } catch (const std::exception&) { threw = true; }
    if (!threw) { std::cerr << "FAIL: expected exception for dataLimite >= dataTrabalho" << std::endl; return 1; }

    std::cout << "OK - test-create-validators" << std::endl;
    return 0;
}


