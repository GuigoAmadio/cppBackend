#pragma once
#include <memory>
#include "../ReconciliationRepository.hpp"
#include "../../../../core/database/ConnectionPool.hpp"

namespace Finance {

class ReconciliationRepositoryImpl : public ReconciliationRepository {
public:
    explicit ReconciliationRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);

    bool save(const Reconciliation& reconciliation) override;
    bool update(const Reconciliation& reconciliation) override;
    bool remove(const std::string& id) override;
    
    std::optional<Reconciliation> findById(const std::string& id, const std::string& tenantId) override;
    std::vector<Reconciliation> findByAccount(const std::string& accountId, const std::string& tenantId) override;
    std::vector<Reconciliation> findByStatus(const std::string& status, const std::string& tenantId) override;
    std::optional<Reconciliation> findLatestByAccount(const std::string& accountId, const std::string& tenantId) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    Reconciliation mapToReconciliation(const Core::Database::QueryResult& result, int row);
};

} // namespace Finance

