#ifndef CREATE_CANDIDATURA_USE_CASE_HPP
#define CREATE_CANDIDATURA_USE_CASE_HPP

#include "../repositories/ICandidaturaRepository.hpp"
#include "../../vagas/repositories/IVagaRepository.hpp"
#include <memory>
#include <stdexcept>

namespace Domains {
namespace Candidaturas {
namespace UseCases {

using Repositories::ICandidaturaRepository;
using Entities::Candidatura;
using Domains::Vagas::Repositories::IVagaRepository;

class CreateCandidaturaUseCase {
private:
    std::shared_ptr<ICandidaturaRepository> candidaturaRepo_;
    std::shared_ptr<IVagaRepository> vagaRepo_;

public:
    CreateCandidaturaUseCase(
        std::shared_ptr<ICandidaturaRepository> candidaturaRepo,
        std::shared_ptr<IVagaRepository> vagaRepo
    ) : candidaturaRepo_(candidaturaRepo), vagaRepo_(vagaRepo) {}

    std::string execute(const Candidatura& candidatura) {
        // 1. Validar se vaga existe e está aberta
        auto vaga = vagaRepo_->findById(candidatura.getVagaId());
        if (!vaga) {
            throw std::runtime_error("Vaga não encontrada");
        }
        
        if (vaga->getStatus() != "aberta") {
            throw std::runtime_error("Vaga não está aberta para candidaturas");
        }
        
        // 2. Verificar se freelancer já se candidatou
        if (candidaturaRepo_->existsCandidatura(candidatura.getVagaId(), candidatura.getFreelancerId())) {
            throw std::runtime_error("Freelancer já se candidatou a esta vaga");
        }
        
        // 3. Criar candidatura
        return candidaturaRepo_->create(candidatura);
    }
};

} // namespace UseCases
} // namespace Candidaturas
} // namespace Domains

#endif // CREATE_CANDIDATURA_USE_CASE_HPP

