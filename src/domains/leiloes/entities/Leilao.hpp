#pragma once

#include <string>
#include <optional>

namespace Domains::Leiloes::Entities {

class Leilao {
public:
    Leilao() = default;

    Leilao(
        const std::string& id,
        const std::string& restauranteId,
        const std::string& titulo,
        const std::string& categoria,
        double valorIdeal,
        double valorMaxQuero,
        const std::string& dataTrabalho,
        std::optional<double> duracaoHoras,
        const std::string& dataLimiteOffers
    ) : id_(id), restauranteId_(restauranteId), titulo_(titulo), categoria_(categoria),
        valorIdeal_(valorIdeal), valorMaxQuero_(valorMaxQuero), dataTrabalho_(dataTrabalho),
        duracaoHoras_(duracaoHoras), dataLimiteOffers_(dataLimiteOffers),
        vagasDisponiveis_(1), status_("aberto") {}

    // Getters básicos
    const std::string& getId() const { return id_; }
    const std::string& getRestauranteId() const { return restauranteId_; }
    const std::string& getTitulo() const { return titulo_; }
    const std::string& getCategoria() const { return categoria_; }
    double getValorIdeal() const { return valorIdeal_; }
    double getValorMaxQuero() const { return valorMaxQuero_; }
    const std::string& getDataTrabalho() const { return dataTrabalho_; }
    const std::optional<double>& getDuracaoHoras() const { return duracaoHoras_; }
    const std::string& getDataLimiteOffers() const { return dataLimiteOffers_; }
    int getVagasDisponiveis() const { return vagasDisponiveis_; }
    const std::string& getStatus() const { return status_; }

    // Regras principais
    bool estaAberto(const std::string& nowIso8601) const {
        return status_ == "aberto" && nowIso8601 < dataLimiteOffers_;
    }

    bool aceitaNovasOffers(const std::string& nowIso8601) const {
        return estaAberto(nowIso8601);
    }

    void fechar() { status_ = "fechado"; }
    void cancelar() { status_ = "cancelado"; }
    void concluir() { status_ = "concluido"; }

private:
    std::string id_;
    std::string restauranteId_;
    std::string titulo_;
    std::string categoria_;
    double valorIdeal_;
    double valorMaxQuero_;
    std::string dataTrabalho_;
    std::optional<double> duracaoHoras_;
    std::string dataLimiteOffers_;
    int vagasDisponiveis_;
    std::string status_; // aberto, fechado, cancelado, concluido
};

} // namespace Domains::Leiloes::Entities


