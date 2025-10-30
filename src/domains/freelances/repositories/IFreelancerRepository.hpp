#pragma once

#include "../entities/FreelancerProfile.hpp"
#include <vector>
#include <optional>
#include <memory>

namespace Domains::Freelances::Repositories {

using Domains::Freelances::Entities::FreelancerProfile;

/**
 * @brief Interface para repositório de freelancers
 */
class IFreelancerRepository {
public:
    virtual ~IFreelancerRepository() = default;

    virtual std::string create(const FreelancerProfile& freelancer) = 0;
    virtual std::optional<FreelancerProfile> findById(const std::string& id) = 0;
    virtual std::optional<FreelancerProfile> findByUserId(const std::string& userId) = 0;
    virtual std::optional<FreelancerProfile> findByCpf(const std::string& cpf) = 0;
    virtual std::vector<FreelancerProfile> findAll() = 0;
    virtual std::vector<FreelancerProfile> findByEspecialidade(const std::string& especialidade) = 0;
    virtual std::vector<FreelancerProfile> findAtivos() = 0;
    virtual bool update(const FreelancerProfile& freelancer) = 0;
    virtual bool deleteById(const std::string& id) = 0;
    virtual std::vector<FreelancerProfile> findNearby(double latitude, double longitude, double radiusKm) = 0;
};

} // namespace Domains::Freelances::Repositories

