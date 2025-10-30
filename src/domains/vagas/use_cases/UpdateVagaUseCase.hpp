#pragma once

#include "../entities/Vaga.hpp"
#include "../repositories/IVagaRepository.hpp"
#include <memory>
#include <stdexcept>

namespace Domains::Vagas::UseCases {

using Domains::Vagas::Entities::Vaga;
using Domains::Vagas::Repositories::IVagaRepository;

class UpdateVagaUseCase {
public:
    explicit UpdateVagaUseCase(std::shared_ptr<IVagaRepository> repository) : repository_(repository) {}
    
    bool execute(const Vaga& vaga) {
        auto existing = repository_->findById(vaga.getId());
        if (!existing.has_value()) {
            throw std::runtime_error("Vaga não encontrada");
        }
        return repository_->update(vaga);
    }

private:
    std::shared_ptr<IVagaRepository> repository_;
};

} // namespace Domains::Vagas::UseCases

