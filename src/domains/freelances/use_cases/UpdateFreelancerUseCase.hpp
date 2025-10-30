#pragma once

#include "../entities/FreelancerProfile.hpp"
#include "../repositories/IFreelancerRepository.hpp"
#include <memory>
#include <stdexcept>

namespace Domains::Freelances::UseCases {

using Domains::Freelances::Entities::FreelancerProfile;
using Domains::Freelances::Repositories::IFreelancerRepository;

/**
 * @brief Use Case para atualizar perfil de freelancer
 */
class UpdateFreelancerUseCase {
public:
    explicit UpdateFreelancerUseCase(std::shared_ptr<IFreelancerRepository> repository)
        : repository_(repository) {}

    bool execute(const FreelancerProfile& freelancer) {
        // Verificar se freelancer existe
        auto existing = repository_->findById(freelancer.getId());
        if (!existing.has_value()) {
            throw std::runtime_error("Freelancer não encontrado");
        }

        return repository_->update(freelancer);
    }

private:
    std::shared_ptr<IFreelancerRepository> repository_;
};

} // namespace Domains::Freelances::UseCases

