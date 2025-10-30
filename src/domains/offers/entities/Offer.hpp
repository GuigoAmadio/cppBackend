#pragma once

#include <string>
#include <optional>

namespace Domains::Offers::Entities {

class Offer {
public:
    Offer() = default;

    Offer(
        const std::string& id,
        const std::string& userId,
        const std::string& restauranteId,
        const std::optional<std::string>& leilaoId,
        double valorOferecido,
        double valorMinimoAceito,
        const std::string& mensagem,
        const std::string& createdAt
    ) : id_(id), userId_(userId), restauranteId_(restauranteId), leilaoId_(leilaoId),
        valorOferecido_(valorOferecido), valorMinimoAceito_(valorMinimoAceito),
        mensagem_(mensagem), status_("pendente"), createdAt_(createdAt) {}

    const std::string& getId() const { return id_; }
    const std::string& getUserId() const { return userId_; }
    const std::string& getRestauranteId() const { return restauranteId_; }
    const std::optional<std::string>& getLeilaoId() const { return leilaoId_; }
    double getValorOferecido() const { return valorOferecido_; }
    double getValorMinimoAceito() const { return valorMinimoAceito_; }
    const std::string& getMensagem() const { return mensagem_; }
    const std::string& getStatus() const { return status_; }
    const std::string& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getRespondedAt() const { return respondedAt_; }

    bool ehDeLeilao() const { return leilaoId_.has_value(); }
    bool ehNaoSolicitada() const { return !leilaoId_.has_value(); }
    bool isPendente() const { return status_ == "pendente"; }

    void aceitar() { status_ = "aceita"; }
    void rejeitar() { status_ = "rejeitada"; }
    void cancelar() { status_ = "cancelada"; }

private:
    std::string id_;
    std::string userId_;
    std::string restauranteId_;
    std::optional<std::string> leilaoId_; // null -> nao_solicitada
    double valorOferecido_;
    double valorMinimoAceito_;
    std::string mensagem_;
    std::string status_; // pendente, aceita, rejeitada, cancelada
    std::string createdAt_;
    std::optional<std::string> respondedAt_;
};

} // namespace Domains::Offers::Entities


