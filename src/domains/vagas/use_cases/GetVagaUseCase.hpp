#pragma once

#include "../entities/Vaga.hpp"
#include "../repositories/IVagaRepository.hpp"
#include <memory>
#include <optional>

namespace Domains::Vagas::UseCases {

using Domains::Vagas::Entities::Vaga;
using Domains::Vagas::Repositories::IVagaRepository;

class GetVagaUseCase {
public:
    explicit GetVagaUseCase(std::shared_ptr<IVagaRepository> repository) : repository_(repository) {}
    
    std::optional<Vaga> executeById(const std::string& id) {
        return repository_->findById(id);
    }

private:
    std::shared_ptr<IVagaRepository> repository_;
};

} // namespace Domains::Vagas::UseCases

