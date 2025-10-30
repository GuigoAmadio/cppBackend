#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

struct OfferProfileProjection {
    std::string offerId;
    std::string freelancerId;
    std::string nome;
    double score;
    int numAvaliacoes;
    double distanciaKm;
    bool shortlisted;
    std::string createdAt; // ISO string for tie-break
};

int main() {
    std::vector<OfferProfileProjection> v{
        {"o1","f1","A",4.5, 10, 3.2, true,  "2025-01-01T10:00:00Z"},
        {"o2","f2","B",4.8,  5, 4.0, true,  "2025-01-01T11:00:00Z"},
        {"o3","f3","C",4.8, 20, 6.0, true,  "2025-01-01T12:00:00Z"},
        {"o4","f4","D",4.2, 30, 2.0, false, "2025-01-01T09:00:00Z"}
    };

    // Comparator igual ao do scheduler: score desc, aval desc, distancia asc, createdAt asc
    std::sort(v.begin(), v.end(), [](const auto& a, const auto& b){
        if (a.score != b.score) return a.score > b.score;
        if (a.numAvaliacoes != b.numAvaliacoes) return a.numAvaliacoes > b.numAvaliacoes;
        if (a.distanciaKm != b.distanciaKm) return a.distanciaKm < b.distanciaKm;
        return a.createdAt < b.createdAt;
    });

    // Esperado: o3 (4.8, 20) > o2 (4.8, 5) > o1 (4.5, 10) > o4 (4.2, 30)
    if (v[0].offerId != "o3" || v[1].offerId != "o2" || v[2].offerId != "o1" || v[3].offerId != "o4") {
        std::cerr << "FAIL ranking order" << std::endl;
        return 1;
    }
    std::cout << "OK - test-ranking" << std::endl;
    return 0;
}


