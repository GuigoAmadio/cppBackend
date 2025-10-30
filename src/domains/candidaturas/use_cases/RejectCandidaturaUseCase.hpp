#ifndef REJECT_CANDIDATURA_USE_CASE_HPP
#define REJECT_CANDIDATURA_USE_CASE_HPP

#include "../repositories/ICandidaturaRepository.hpp"
#include <memory>
#include <stdexcept>

namespace Domains {
namespace Candidaturas {
namespace UseCases {

using Repositories::ICandidaturaRepository;
using Entities::Candidatura;

class RejectCandidaturaUseCase {
private:
    std::shared_ptr<ICandidaturaRepository> candidaturaRepo_;

public:
    explicit RejectCandidaturaUseCase(std::shared_ptr<ICandidaturaRepository> candidaturaRepo)
        : candidaturaRepo_(candidaturaRepo) {}

    void execute(const std::string& candidaturaId, const std::string& motivo) {
        // 1. Buscar candidatura
        auto candidatura = candidaturaRepo_->findById(candidaturaId);
        if (!candidatura) {
            throw std::runtime_error("Candidatura não encontrada");
        }
        
        // 2. Validar status e rejeitar
        candidatura->rejeitar(motivo);
        candidatura->setRespondedAt("NOW()");
        
        // 3. Atualizar no banco
        candidaturaRepo_->update(*candidatura);
    }
};

} // namespace UseCases
} // namespace Candidaturas
} // namespace Domains

#endif // REJECT_CANDIDATURA_USE_CASE_HPP

