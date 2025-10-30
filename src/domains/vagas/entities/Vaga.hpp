#pragma once

#include <string>
#include <optional>

namespace Domains::Vagas::Entities {

/**
 * @brief Entity representando uma vaga de trabalho
 */
class Vaga {
public:
    Vaga() = default;
    
    Vaga(
        const std::string& id,
        const std::string& restauranteId,
        const std::string& titulo,
        const std::string& categoria,
        double precoOferecido,
        const std::string& dataTrabalho
    ) : id_(id),
        restauranteId_(restauranteId),
        titulo_(titulo),
        categoria_(categoria),
        precoOferecido_(precoOferecido),
        dataTrabalho_(dataTrabalho),
        vagasDisponiveis_(1),
        vagasPreenchidas_(0),
        status_("aberta") {}

    // Getters
    std::string getId() const { return id_; }
    std::string getRestauranteId() const { return restauranteId_; }
    std::string getTitulo() const { return titulo_; }
    std::optional<std::string> getDescricao() const { return descricao_; }
    std::string getCategoria() const { return categoria_; }
    double getPrecoOferecido() const { return precoOferecido_; }
    std::string getDataTrabalho() const { return dataTrabalho_; }
    std::optional<std::string> getHorarioInicio() const { return horarioInicio_; }
    std::optional<std::string> getHorarioFim() const { return horarioFim_; }
    std::optional<double> getDuracaoHoras() const { return duracaoHoras_; }
    std::optional<std::string> getDataLimiteCandidatura() const { return dataLimiteCandidatura_; }
    std::optional<std::string> getRequisitos() const { return requisitos_; }
    int getVagasDisponiveis() const { return vagasDisponiveis_; }
    int getVagasPreenchidas() const { return vagasPreenchidas_; }
    std::string getStatus() const { return status_; }
    std::string getCreatedAt() const { return createdAt_; }
    std::string getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setId(const std::string& id) { id_ = id; }
    void setRestauranteId(const std::string& restauranteId) { restauranteId_ = restauranteId; }
    void setTitulo(const std::string& titulo) { titulo_ = titulo; }
    void setDescricao(const std::string& descricao) { descricao_ = descricao; }
    void setCategoria(const std::string& categoria) { categoria_ = categoria; }
    void setPrecoOferecido(double preco) { precoOferecido_ = preco; }
    void setDataTrabalho(const std::string& data) { dataTrabalho_ = data; }
    void setHorarioInicio(const std::string& horario) { horarioInicio_ = horario; }
    void setHorarioFim(const std::string& horario) { horarioFim_ = horario; }
    void setDuracaoHoras(double duracao) { duracaoHoras_ = duracao; }
    void setDataLimiteCandidatura(const std::string& data) { dataLimiteCandidatura_ = data; }
    void setRequisitos(const std::string& requisitos) { requisitos_ = requisitos; }
    void setVagasDisponiveis(int vagas) { vagasDisponiveis_ = vagas; }
    void setVagasPreenchidas(int vagas) { vagasPreenchidas_ = vagas; }
    void setStatus(const std::string& status) { status_ = status; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }

    // Business logic
    bool isAberta() const {
        return status_ == "aberta";
    }

    bool temVagasDisponiveis() const {
        return vagasPreenchidas_ < vagasDisponiveis_;
    }

    void incrementarVagasPreenchidas() {
        if (temVagasDisponiveis()) {
            vagasPreenchidas_++;
            if (vagasPreenchidas_ >= vagasDisponiveis_) {
                fechar();
            }
        }
    }

    void decrementarVagasPreenchidas() {
        if (vagasPreenchidas_ > 0) {
            vagasPreenchidas_--;
            if (status_ == "fechada") {
                status_ = "aberta";
            }
        }
    }

    void fechar() {
        status_ = "fechada";
    }

    void cancelar() {
        status_ = "cancelada";
    }

    void reabrir() {
        if (status_ == "fechada") {
            status_ = "aberta";
        }
    }

private:
    std::string id_;
    std::string restauranteId_;
    std::string titulo_;
    std::optional<std::string> descricao_;
    std::string categoria_;
    double precoOferecido_;
    std::string dataTrabalho_;
    std::optional<std::string> horarioInicio_;
    std::optional<std::string> horarioFim_;
    std::optional<double> duracaoHoras_;
    std::optional<std::string> dataLimiteCandidatura_;
    std::optional<std::string> requisitos_;  // JSON string
    int vagasDisponiveis_;
    int vagasPreenchidas_;
    std::string status_;  // aberta, fechada, cancelada
    std::string createdAt_;
    std::string updatedAt_;
};

} // namespace Domains::Vagas::Entities

