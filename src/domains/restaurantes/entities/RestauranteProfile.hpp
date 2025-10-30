#pragma once

#include <string>
#include <optional>
#include "../value_objects/Coordenadas.hpp"

namespace Domains::Restaurantes::Entities {

/**
 * @brief Entity representando o perfil de um restaurante/empresa
 */
class RestauranteProfile {
public:
    RestauranteProfile() = default;
    
    RestauranteProfile(
        const std::string& id,
        const std::string& userId,
        const std::string& cnpj,
        const std::string& razaoSocial,
        double latitude,
        double longitude
    ) : id_(id), 
        userId_(userId), 
        cnpj_(cnpj), 
        razaoSocial_(razaoSocial),
        coordenadas_(latitude, longitude),
        documentosVerificados_(false),
        ativo_(true) {}

    // Getters
    std::string getId() const { return id_; }
    std::string getUserId() const { return userId_; }
    std::string getCnpj() const { return cnpj_; }
    std::string getRazaoSocial() const { return razaoSocial_; }
    std::optional<std::string> getNomeFantasia() const { return nomeFantasia_; }
    std::optional<std::string> getDescricao() const { return descricao_; }
    std::optional<std::string> getCategoria() const { return categoria_; }
    std::optional<std::string> getLogo() const { return logo_; }
    ValueObjects::Coordenadas getCoordenadas() const { return coordenadas_; }
    std::optional<std::string> getEnderecoCompleto() const { return enderecoCompleto_; }
    std::optional<std::string> getCep() const { return cep_; }
    std::optional<std::string> getCidade() const { return cidade_; }
    std::optional<std::string> getEstado() const { return estado_; }
    std::optional<std::string> getHorarioFuncionamento() const { return horarioFuncionamento_; }
    bool isDocumentosVerificados() const { return documentosVerificados_; }
    bool isAtivo() const { return ativo_; }
    std::string getCreatedAt() const { return createdAt_; }
    std::string getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setId(const std::string& id) { id_ = id; }
    void setUserId(const std::string& userId) { userId_ = userId; }
    void setCnpj(const std::string& cnpj) { cnpj_ = cnpj; }
    void setRazaoSocial(const std::string& razaoSocial) { razaoSocial_ = razaoSocial; }
    void setNomeFantasia(const std::string& nomeFantasia) { nomeFantasia_ = nomeFantasia; }
    void setDescricao(const std::string& descricao) { descricao_ = descricao; }
    void setCategoria(const std::string& categoria) { categoria_ = categoria; }
    void setLogo(const std::string& logo) { logo_ = logo; }
    void setCoordenadas(const ValueObjects::Coordenadas& coords) { coordenadas_ = coords; }
    void setEnderecoCompleto(const std::string& endereco) { enderecoCompleto_ = endereco; }
    void setCep(const std::string& cep) { cep_ = cep; }
    void setCidade(const std::string& cidade) { cidade_ = cidade; }
    void setEstado(const std::string& estado) { estado_ = estado; }
    void setHorarioFuncionamento(const std::string& horario) { horarioFuncionamento_ = horario; }
    void setDocumentosVerificados(bool verificados) { documentosVerificados_ = verificados; }
    void setAtivo(bool ativo) { ativo_ = ativo; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }

    // Business logic
    bool podePostarVaga() const {
        return ativo_ && documentosVerificados_;
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
    std::string cnpj_;
    std::string razaoSocial_;
    std::optional<std::string> nomeFantasia_;
    std::optional<std::string> descricao_;
    std::optional<std::string> categoria_;
    std::optional<std::string> logo_;
    ValueObjects::Coordenadas coordenadas_;
    std::optional<std::string> enderecoCompleto_;
    std::optional<std::string> cep_;
    std::optional<std::string> cidade_;
    std::optional<std::string> estado_;
    std::optional<std::string> horarioFuncionamento_;  // JSON string
    bool documentosVerificados_;
    bool ativo_;
    std::string createdAt_;
    std::string updatedAt_;
};

} // namespace Domains::Restaurantes::Entities

