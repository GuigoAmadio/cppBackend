#pragma once

#include "IVagaRepository.hpp"
#include "../../../core/database/ConnectionPool.hpp"
#include <memory>

namespace Domains::Vagas::Repositories {

using Core::Database::ConnectionPool;
using Core::Database::QueryResult;

class VagaRepository : public IVagaRepository {
public:
    explicit VagaRepository(std::shared_ptr<ConnectionPool> pool) : pool_(pool) {}

    std::string create(const Vaga& vaga) override;
    std::optional<Vaga> findById(const std::string& id) override;
    std::vector<Vaga> findByRestaurante(const std::string& restauranteId) override;
    std::vector<Vaga> findByCategoria(const std::string& categoria) override;
    std::vector<Vaga> findByStatus(const std::string& status) override;
    std::vector<Vaga> findAbertas() override;
    std::vector<Vaga> findAll() override;
    bool update(const Vaga& vaga) override;
    bool deleteById(const std::string& id) override;

private:
    std::shared_ptr<ConnectionPool> pool_;
    Vaga mapToVaga(const QueryResult& result, int row);
};

} // namespace Domains::Vagas::Repositories

