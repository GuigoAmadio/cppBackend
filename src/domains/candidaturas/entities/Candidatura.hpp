#ifndef CANDIDATURA_HPP
#define CANDIDATURA_HPP

#include <string>
#include <optional>
#include <stdexcept>

namespace Domains {
namespace Candidaturas {
namespace Entities {

class Candidatura {
private:
    std::string id_;
    std::string vagaId_;
    std::string freelancerId_;
    std::optional<double> precoPropostoFreelancer_;
    std::optional<std::string> mensagem_;
    std::string status_;  // pendente, aceita, rejeitada, cancelada
    std::optional<std::string> motivoRejeicao_;
    std::string createdAt_;
    std::optional<std::string> respondedAt_;

public:
    Candidatura() = default;
    
    Candidatura(
        const std::string& id,
        const std::string& vagaId,
        const std::string& freelancerId,
        const std::optional<double>& precoPropostoFreelancer,
        const std::optional<std::string>& mensagem,
        const std::string& status,
        const std::optional<std::string>& motivoRejeicao,
        const std::string& createdAt,
        const std::optional<std::string>& respondedAt
    ) : id_(id), vagaId_(vagaId), freelancerId_(freelancerId),
        precoPropostoFreelancer_(precoPropostoFreelancer),
        mensagem_(mensagem), status_(status),
        motivoRejeicao_(motivoRejeicao),
        createdAt_(createdAt), respondedAt_(respondedAt) {}

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getVagaId() const { return vagaId_; }
    const std::string& getFreelancerId() const { return freelancerId_; }
    const std::optional<double>& getPrecoPropostoFreelancer() const { return precoPropostoFreelancer_; }
    const std::optional<std::string>& getMensagem() const { return mensagem_; }
    const std::string& getStatus() const { return status_; }
    const std::optional<std::string>& getMotivoRejeicao() const { return motivoRejeicao_; }
    const std::string& getCreatedAt() const { return createdAt_; }
    const std::optional<std::string>& getRespondedAt() const { return respondedAt_; }

    // Setters
    void setStatus(const std::string& status) { status_ = status; }
    void setMotivoRejeicao(const std::string& motivo) { motivoRejeicao_ = motivo; }
    void setRespondedAt(const std::string& respondedAt) { respondedAt_ = respondedAt; }

    // Business logic
    bool isPendente() const { return status_ == "pendente"; }
    bool isAceita() const { return status_ == "aceita"; }
    bool isRejeitada() const { return status_ == "rejeitada"; }
    
    void aceitar() {
        if (!isPendente()) {
            throw std::runtime_error("Só é possível aceitar candidaturas pendentes");
        }
        status_ = "aceita";
    }
    
    void rejeitar(const std::string& motivo) {
        if (!isPendente()) {
            throw std::runtime_error("Só é possível rejeitar candidaturas pendentes");
        }
        status_ = "rejeitada";
        motivoRejeicao_ = motivo;
    }
};

} // namespace Entities
} // namespace Candidaturas
} // namespace Domains

#endif // CANDIDATURA_HPP
