#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "../entities/Reconciliation.hpp"

namespace Finance {

class ReconciliationRepository {
public:
    virtual ~ReconciliationRepository() = default;

    virtual bool save(const Reconciliation& reconciliation) = 0;
    virtual bool update(const Reconciliation& reconciliation) = 0;
    virtual bool remove(const std::string& id) = 0;
    
    virtual std::optional<Reconciliation> findById(const std::string& id, const std::string& tenantId) = 0;
    virtual std::vector<Reconciliation> findByAccount(const std::string& accountId, const std::string& tenantId) = 0;
    virtual std::vector<Reconciliation> findByStatus(const std::string& status, const std::string& tenantId) = 0;
    virtual std::optional<Reconciliation> findLatestByAccount(const std::string& accountId, const std::string& tenantId) = 0;
};

} // namespace Finance

