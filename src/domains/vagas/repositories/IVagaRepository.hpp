#pragma once

#include "../entities/Vaga.hpp"
#include <vector>
#include <optional>
#include <memory>

namespace Domains::Vagas::Repositories {

using Domains::Vagas::Entities::Vaga;

class IVagaRepository {
public:
    virtual ~IVagaRepository() = default;

    virtual std::string create(const Vaga& vaga) = 0;
    virtual std::optional<Vaga> findById(const std::string& id) = 0;
    virtual std::vector<Vaga> findByRestaurante(const std::string& restauranteId) = 0;
    virtual std::vector<Vaga> findByCategoria(const std::string& categoria) = 0;
    virtual std::vector<Vaga> findByStatus(const std::string& status) = 0;
    virtual std::vector<Vaga> findAbertas() = 0;
    virtual std::vector<Vaga> findAll() = 0;
    virtual bool update(const Vaga& vaga) = 0;
    virtual bool deleteById(const std::string& id) = 0;
};

} // namespace Domains::Vagas::Repositories

