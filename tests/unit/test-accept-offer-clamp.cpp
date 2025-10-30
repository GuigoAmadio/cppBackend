#include <iostream>
#include <stdexcept>
#include "../../src/domains/offers/use_cases/AcceptOfferUseCase.hpp"

using Domains::Offers::UseCases::AcceptOfferUseCase;

static void assertEqual(double a, double b, const char* msg) {
    if (std::abs(a - b) > 1e-9) {
        std::cerr << "FAIL: " << msg << " (expected=" << b << ", got=" << a << ")\n";
        std::exit(1);
    }
}

int main() {
    // oferecido dentro do intervalo
    assertEqual(AcceptOfferUseCase::clampPreco(130, 120, 180), 130, "clamp middle");
    // abaixo do mínimo do freelancer
    assertEqual(AcceptOfferUseCase::clampPreco(100, 120, 180), 120, "clamp to min freelancer");
    // acima do máximo do restaurante
    assertEqual(AcceptOfferUseCase::clampPreco(200, 120, 180), 180, "clamp to max restaurante");
    // bordas
    assertEqual(AcceptOfferUseCase::clampPreco(120, 120, 180), 120, "border min");
    assertEqual(AcceptOfferUseCase::clampPreco(180, 120, 180), 180, "border max");
    std::cout << "OK - test-accept-offer-clamp" << std::endl;
    return 0;
}


