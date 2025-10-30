#pragma once
#include "../entities/Candidatura.hpp"
#include "../repositories/ICandidaturaRepository.hpp"
#include <memory>

namespace Domains::Candidaturas::UseCases {
using Domains::Candidaturas::Entities::Candidatura;
using Domains::Candidaturas::Repositories::ICandidaturaRepository;

class SubmitCandidaturaUseCase {
public:
    explicit SubmitCandidaturaUseCase(std::shared_ptr<ICandidaturaRepository> repository)
        : repository_(repository) {}
    
    std::string execute(const Candidatura& candidatura) {
        // Validar: freelancer não pode se candidatar 2x na mesma vaga
        auto existing = repository_->findByVagaAndFreelancer(candidatura.getVagaId(), candidatura.getFreelancerId());
        if (existing.has_value()) {
            throw std::runtime_error("Você já se candidatou para esta vaga");
        }
        
        // TODO: Validar se vaga está aberta
        // TODO: Validar se freelancer existe
        
        return repository_->create(candidatura);
    }

private:
    std::shared_ptr<ICandidaturaRepository> repository_;
};

} // namespace Domains::Candidaturas::UseCases

