#ifndef ICANDIDATURA_REPOSITORY_HPP
#define ICANDIDATURA_REPOSITORY_HPP

#include <string>
#include <vector>
#include <optional>
#include "../entities/Candidatura.hpp"

namespace Domains {
namespace Candidaturas {
namespace Repositories {

using Entities::Candidatura;

class ICandidaturaRepository {
public:
    virtual ~ICandidaturaRepository() = default;
    
    virtual std::string create(const Candidatura& candidatura) = 0;
    virtual std::optional<Candidatura> findById(const std::string& id) = 0;
    virtual std::vector<Candidatura> findByVaga(const std::string& vagaId) = 0;
    virtual std::vector<Candidatura> findByFreelancer(const std::string& freelancerId) = 0;
    virtual bool existsCandidatura(const std::string& vagaId, const std::string& freelancerId) = 0;
    virtual void update(const Candidatura& candidatura) = 0;
    virtual int countPendentesByVaga(const std::string& vagaId) = 0;
};

} // namespace Repositories
} // namespace Candidaturas
} // namespace Domains

#endif // ICANDIDATURA_REPOSITORY_HPP
