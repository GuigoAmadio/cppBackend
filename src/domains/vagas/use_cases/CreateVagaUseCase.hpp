#pragma once

#include "../entities/Vaga.hpp"
#include "../repositories/IVagaRepository.hpp"
#include <memory>

namespace Domains::Vagas::UseCases {

using Domains::Vagas::Entities::Vaga;
using Domains::Vagas::Repositories::IVagaRepository;

class CreateVagaUseCase {
public:
    explicit CreateVagaUseCase(std::shared_ptr<IVagaRepository> repository) : repository_(repository) {}
    
    std::string execute(const Vaga& vaga) {
        // TODO: Validar se restaurante existe
        // TODO: Validar data de trabalho no futuro
        return repository_->create(vaga);
    }

private:
    std::shared_ptr<IVagaRepository> repository_;
};

} // namespace Domains::Vagas::UseCases

