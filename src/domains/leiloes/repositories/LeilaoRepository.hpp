#pragma once

#include <string>
#include <optional>
#include <vector>
#include "../entities/Leilao.hpp"
#include "../../../core/database/ConnectionPool.hpp"

namespace Domains::Leiloes::Repositories {

using Domains::Leiloes::Entities::Leilao;
using Core::Database::ConnectionPool;

class LeilaoRepository {
public:
    explicit LeilaoRepository(std::shared_ptr<ConnectionPool> pool) : pool_(std::move(pool)) {}

    std::optional<Leilao> findById(const std::string& id);
    std::vector<Leilao> listOpen(const std::string& nowIso8601);
    std::vector<Leilao> listExpiredPendingClose(const std::string& nowIso8601);
    void save(const Leilao& leilao);
    void close(const std::string& id);
    std::string generateId();

private:
    std::shared_ptr<ConnectionPool> pool_;
};

} // namespace Domains::Leiloes::Repositories


