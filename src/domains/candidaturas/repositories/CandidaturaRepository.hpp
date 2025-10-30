#ifndef CANDIDATURA_REPOSITORY_HPP
#define CANDIDATURA_REPOSITORY_HPP

#include "ICandidaturaRepository.hpp"
#include "../../../core/database/ConnectionPool.hpp"
#include <memory>

namespace Domains {
namespace Candidaturas {
namespace Repositories {

class CandidaturaRepository : public ICandidaturaRepository {
private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;

public:
    explicit CandidaturaRepository(std::shared_ptr<Core::Database::ConnectionPool> pool);
    
    std::string create(const Candidatura& candidatura) override;
    std::optional<Candidatura> findById(const std::string& id) override;
    std::vector<Candidatura> findByVaga(const std::string& vagaId) override;
    std::vector<Candidatura> findByFreelancer(const std::string& freelancerId) override;
    bool existsCandidatura(const std::string& vagaId, const std::string& freelancerId) override;
    void update(const Candidatura& candidatura) override;
    int countPendentesByVaga(const std::string& vagaId) override;

private:
    Candidatura mapToCandidatura(const Core::Database::QueryResult& result, int row);
};

} // namespace Repositories
} // namespace Candidaturas
} // namespace Domains

#endif // CANDIDATURA_REPOSITORY_HPP
