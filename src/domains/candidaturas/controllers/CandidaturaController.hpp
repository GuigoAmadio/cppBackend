#ifndef CANDIDATURA_CONTROLLER_HPP
#define CANDIDATURA_CONTROLLER_HPP

#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../repositories/CandidaturaRepository.hpp"
#include "../use_cases/CreateCandidaturaUseCase.hpp"
#include "../use_cases/AcceptCandidaturaUseCase.hpp"
#include "../use_cases/RejectCandidaturaUseCase.hpp"
#include "../../vagas/repositories/VagaRepository.hpp"
#include <memory>

namespace Domains {
namespace Candidaturas {
namespace Controllers {

using Core::Http::Request;
using Core::Http::Response;
using Repositories::CandidaturaRepository;
using UseCases::CreateCandidaturaUseCase;
using UseCases::AcceptCandidaturaUseCase;
using UseCases::RejectCandidaturaUseCase;
using Domains::Vagas::Repositories::VagaRepository;

class CandidaturaController {
private:
    std::shared_ptr<CandidaturaRepository> repo_;
    std::shared_ptr<VagaRepository> vagaRepo_;
    std::shared_ptr<CreateCandidaturaUseCase> createUseCase_;
    std::shared_ptr<AcceptCandidaturaUseCase> acceptUseCase_;
    std::shared_ptr<RejectCandidaturaUseCase> rejectUseCase_;

public:
    CandidaturaController(
        std::shared_ptr<CandidaturaRepository> repo,
        std::shared_ptr<VagaRepository> vagaRepo
    );

    // POST /api/candidaturas
    Response create(const Request& req);
    
    // GET /api/candidaturas/:id
    Response getById(const Request& req);
    
    // GET /api/candidaturas/vaga/:vagaId
    Response getByVaga(const Request& req);
    
    // GET /api/candidaturas/freelancer/:freelancerId
    Response getByFreelancer(const Request& req);
    
    // PUT /api/candidaturas/:id/accept
    Response accept(const Request& req);
    
    // PUT /api/candidaturas/:id/reject
    Response reject(const Request& req);
};

} // namespace Controllers
} // namespace Candidaturas
} // namespace Domains

#endif // CANDIDATURA_CONTROLLER_HPP

