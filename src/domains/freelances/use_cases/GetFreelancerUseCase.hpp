#pragma once

#include "../entities/FreelancerProfile.hpp"
#include "../repositories/IFreelancerRepository.hpp"
#include <memory>
#include <optional>

namespace Domains::Freelances::UseCases {

using Domains::Freelances::Entities::FreelancerProfile;
using Domains::Freelances::Repositories::IFreelancerRepository;

/**
 * @brief Use Case para buscar freelancer
 */
class GetFreelancerUseCase {
public:
    explicit GetFreelancerUseCase(std::shared_ptr<IFreelancerRepository> repository)
        : repository_(repository) {}

    std::optional<FreelancerProfile> executeById(const std::string& id) {
        return repository_->findById(id);
    }

    std::optional<FreelancerProfile> executeByUserId(const std::string& userId) {
        return repository_->findByUserId(userId);
    }

    std::optional<FreelancerProfile> executeByCpf(const std::string& cpf) {
        return repository_->findByCpf(cpf);
    }

private:
    std::shared_ptr<IFreelancerRepository> repository_;
};

} // namespace Domains::Freelances::UseCases

