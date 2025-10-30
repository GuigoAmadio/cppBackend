#pragma once

#include "../entities/FreelancerProfile.hpp"
#include "../repositories/IFreelancerRepository.hpp"
#include <memory>
#include <stdexcept>

namespace Domains::Freelances::UseCases {

using Domains::Freelances::Entities::FreelancerProfile;
using Domains::Freelances::Repositories::IFreelancerRepository;

/**
 * @brief Use Case para criar perfil de freelancer
 */
class CreateFreelancerUseCase {
public:
    explicit CreateFreelancerUseCase(std::shared_ptr<IFreelancerRepository> repository)
        : repository_(repository) {}

    std::string execute(const FreelancerProfile& freelancer) {
        // Validação: verificar se user_id já tem freelancer
        auto existingByUser = repository_->findByUserId(freelancer.getUserId());
        if (existingByUser.has_value()) {
            throw std::runtime_error("User já possui um perfil de freelancer");
        }

        // Validação: verificar se CPF já existe (se fornecido)
        if (freelancer.getCpf().has_value()) {
            auto existingByCpf = repository_->findByCpf(*freelancer.getCpf());
            if (existingByCpf.has_value()) {
                throw std::runtime_error("CPF já cadastrado");
            }
        }

        // Validação: especialidades são obrigatórias
        if (freelancer.getEspecialidades().empty()) {
            throw std::invalid_argument("Pelo menos uma especialidade é obrigatória");
        }

        return repository_->create(freelancer);
    }

private:
    std::shared_ptr<IFreelancerRepository> repository_;
};

} // namespace Domains::Freelances::UseCases

