#ifndef ACCEPT_CANDIDATURA_USE_CASE_HPP
#define ACCEPT_CANDIDATURA_USE_CASE_HPP

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

class AcceptCandidaturaUseCase {
private:
    std::shared_ptr<ICandidaturaRepository> candidaturaRepo_;
    std::shared_ptr<IVagaRepository> vagaRepo_;

public:
    AcceptCandidaturaUseCase(
        std::shared_ptr<ICandidaturaRepository> candidaturaRepo,
        std::shared_ptr<IVagaRepository> vagaRepo
    ) : candidaturaRepo_(candidaturaRepo), vagaRepo_(vagaRepo) {}

    void execute(const std::string& candidaturaId) {
        // 1. Buscar candidatura
        auto candidatura = candidaturaRepo_->findById(candidaturaId);
        if (!candidatura) {
            throw std::runtime_error("Candidatura não encontrada");
        }
        
        // 2. Validar status
        if (!candidatura->isPendente()) {
            throw std::runtime_error("Só é possível aceitar candidaturas pendentes");
        }
        
        // 3. Buscar vaga
        auto vaga = vagaRepo_->findById(candidatura->getVagaId());
        if (!vaga) {
            throw std::runtime_error("Vaga não encontrada");
        }
        
        // 4. Verificar se há vagas disponíveis
        if (vaga->getVagasDisponiveis() <= 0) {
            throw std::runtime_error("Não há mais vagas disponíveis");
        }
        
        // 5. Aceitar candidatura
        candidatura->aceitar();
        candidatura->setRespondedAt("NOW()");  // Será convertido no SQL
        candidaturaRepo_->update(*candidatura);
        
        // 6. Decrementar vagas disponíveis e incrementar preenchidas
        vaga->setVagasPreenchidas(vaga->getVagasPreenchidas() + 1);
        vaga->setVagasDisponiveis(vaga->getVagasDisponiveis() - 1);
        
        // 7. Se não há mais vagas, fechar
        if (vaga->getVagasDisponiveis() == 0) {
            vaga->setStatus("fechada");
        }
        
        vagaRepo_->update(*vaga);
        
        // TODO: Criar JobAgreed (será implementado no próximo módulo)
        // jobAgreedRepo_->create(...);
    }
};

} // namespace UseCases
} // namespace Candidaturas
} // namespace Domains

#endif // ACCEPT_CANDIDATURA_USE_CASE_HPP
