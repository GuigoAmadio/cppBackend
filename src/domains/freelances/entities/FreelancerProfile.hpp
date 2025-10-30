#pragma once

#include <string>
#include <optional>
#include <vector>
#include "../value_objects/Coordenadas.hpp"

namespace Domains::Freelances::Entities {

using Domains::Freelances::ValueObjects::Coordenadas;

/**
 * @brief Entity representando o perfil de um freelancer
 */
class FreelancerProfile {
public:
    FreelancerProfile() = default;
    
    FreelancerProfile(
        const std::string& id,
        const std::string& userId,
        const std::vector<std::string>& especialidades
    ) : id_(id), 
        userId_(userId),
        especialidades_(especialidades),
        raioAtuacaoKm_(10.0),
        documentosVerificados_(false),
        ativo_(true) {}

    // Getters
    std::string getId() const { return id_; }
    std::string getUserId() const { return userId_; }
    std::vector<std::string> getEspecialidades() const { return especialidades_; }
    std::optional<int> getExperienciaAnos() const { return experienciaAnos_; }
    std::optional<double> getPrecoMinimo() const { return precoMinimo_; }
    std::optional<std::string> getCpf() const { return cpf_; }
    std::optional<Coordenadas> getCoordenadas() const { return coordenadas_; }
    double getRaioAtuacaoKm() const { return raioAtuacaoKm_; }
    std::optional<std::string> getDisponibilidade() const { return disponibilidade_; }
    bool isDocumentosVerificados() const { return documentosVerificados_; }
    bool isAtivo() const { return ativo_; }
    std::string getCreatedAt() const { return createdAt_; }
    std::string getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setId(const std::string& id) { id_ = id; }
    void setUserId(const std::string& userId) { userId_ = userId; }
    void setEspecialidades(const std::vector<std::string>& esp) { especialidades_ = esp; }
    void setExperienciaAnos(int anos) { experienciaAnos_ = anos; }
    void setPrecoMinimo(double preco) { precoMinimo_ = preco; }
    void setCpf(const std::string& cpf) { cpf_ = cpf; }
    void setCoordenadas(const Coordenadas& coords) { coordenadas_ = coords; }
    void setRaioAtuacaoKm(double raio) { raioAtuacaoKm_ = raio; }
    void setDisponibilidade(const std::string& disp) { disponibilidade_ = disp; }
    void setDocumentosVerificados(bool verificados) { documentosVerificados_ = verificados; }
    void setAtivo(bool ativo) { ativo_ = ativo; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }

    // Business logic
    bool podeCandidatar() const {
        return ativo_ && documentosVerificados_;
    }

    bool temEspecialidade(const std::string& especialidade) const {
        for (const auto& esp : especialidades_) {
            if (esp == especialidade) return true;
        }
        return false;
    }

    void ativar() {
        ativo_ = true;
    }

    void desativar() {
        ativo_ = false;
    }

    void verificarDocumentos() {
        documentosVerificados_ = true;
    }

private:
    std::string id_;
    std::string userId_;
    std::vector<std::string> especialidades_;
    std::optional<int> experienciaAnos_;
    std::optional<double> precoMinimo_;
    std::optional<std::string> cpf_;
    std::optional<Coordenadas> coordenadas_;
    double raioAtuacaoKm_;
    std::optional<std::string> disponibilidade_;  // JSON string
    bool documentosVerificados_;
    bool ativo_;
    std::string createdAt_;
    std::string updatedAt_;
};

} // namespace Domains::Freelances::Entities

