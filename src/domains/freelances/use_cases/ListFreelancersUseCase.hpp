#pragma once

#include "../entities/FreelancerProfile.hpp"
#include "../repositories/IFreelancerRepository.hpp"
#include <memory>
#include <vector>

namespace Domains::Freelances::UseCases {

using Domains::Freelances::Entities::FreelancerProfile;
using Domains::Freelances::Repositories::IFreelancerRepository;

/**
 * @brief Use Case para listar freelancers com filtros
 */
class ListFreelancersUseCase {
public:
    explicit ListFreelancersUseCase(std::shared_ptr<IFreelancerRepository> repository)
        : repository_(repository) {}

    std::vector<FreelancerProfile> executeAll() {
        return repository_->findAll();
    }

    std::vector<FreelancerProfile> executeAtivos() {
        return repository_->findAtivos();
    }

    std::vector<FreelancerProfile> executeByEspecialidade(const std::string& especialidade) {
        return repository_->findByEspecialidade(especialidade);
    }

private:
    std::shared_ptr<IFreelancerRepository> repository_;
};

} // namespace Domains::Freelances::UseCases

