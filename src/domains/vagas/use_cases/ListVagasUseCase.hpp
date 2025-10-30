#pragma once

#include "../entities/Vaga.hpp"
#include "../repositories/IVagaRepository.hpp"
#include <memory>
#include <vector>

namespace Domains::Vagas::UseCases {

using Domains::Vagas::Entities::Vaga;
using Domains::Vagas::Repositories::IVagaRepository;

class ListVagasUseCase {
public:
    explicit ListVagasUseCase(std::shared_ptr<IVagaRepository> repository) : repository_(repository) {}
    
    std::vector<Vaga> executeAll() { return repository_->findAll(); }
    std::vector<Vaga> executeAbertas() { return repository_->findAbertas(); }
    std::vector<Vaga> executeByCategoria(const std::string& cat) { return repository_->findByCategoria(cat); }
    std::vector<Vaga> executeByRestaurante(const std::string& restId) { return repository_->findByRestaurante(restId); }

private:
    std::shared_ptr<IVagaRepository> repository_;
};

} // namespace Domains::Vagas::UseCases

