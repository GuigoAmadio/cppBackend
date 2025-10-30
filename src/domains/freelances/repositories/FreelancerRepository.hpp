#pragma once

#include "IFreelancerRepository.hpp"
#include "../../../core/database/ConnectionPool.hpp"
#include <memory>

namespace Domains::Freelances::Repositories {

using Core::Database::ConnectionPool;
using Core::Database::QueryResult;

/**
 * @brief Implementação do repositório de freelancers
 */
class FreelancerRepository : public IFreelancerRepository {
public:
    explicit FreelancerRepository(std::shared_ptr<ConnectionPool> pool)
        : pool_(pool) {}

    std::string create(const FreelancerProfile& freelancer) override;
    std::optional<FreelancerProfile> findById(const std::string& id) override;
    std::optional<FreelancerProfile> findByUserId(const std::string& userId) override;
    std::optional<FreelancerProfile> findByCpf(const std::string& cpf) override;
    std::vector<FreelancerProfile> findAll() override;
    std::vector<FreelancerProfile> findByEspecialidade(const std::string& especialidade) override;
    std::vector<FreelancerProfile> findAtivos() override;
    bool update(const FreelancerProfile& freelancer) override;
    bool deleteById(const std::string& id) override;
    std::vector<FreelancerProfile> findNearby(double latitude, double longitude, double radiusKm) override;

private:
    std::shared_ptr<ConnectionPool> pool_;
    FreelancerProfile mapToFreelancer(const QueryResult& result, int row);
};

} // namespace Domains::Freelances::Repositories

