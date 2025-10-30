#pragma once
#include "../entities/Candidatura.hpp"
#include "../repositories/ICandidaturaRepository.hpp"
#include <memory>
#include <vector>

namespace Domains::Candidaturas::UseCases {
using Domains::Candidaturas::Entities::Candidatura;
using Domains::Candidaturas::Repositories::ICandidaturaRepository;

class ListCandidaturasUseCase {
public:
    explicit ListCandidaturasUseCase(std::shared_ptr<ICandidaturaRepository> repository)
        : repository_(repository) {}
    
    std::vector<Candidatura> executeByVaga(const std::string& vagaId) {
        return repository_->findByVaga(vagaId);
    }
    
    std::vector<Candidatura> executeByFreelancer(const std::string& freelancerId) {
        return repository_->findByFreelancer(freelancerId);
    }

private:
    std::shared_ptr<ICandidaturaRepository> repository_;
};

} // namespace Domains::Candidaturas::UseCases

