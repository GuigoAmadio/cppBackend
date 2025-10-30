#pragma once
#include <memory>
#include <optional>
#include <vector>
#include <map>
#include "../entities/Transaction.hpp"

namespace Finance {

class TransactionRepository {
public:
    virtual ~TransactionRepository() = default;

    virtual bool save(const Transaction& transaction) = 0;
    virtual bool update(const Transaction& transaction) = 0;
    virtual bool remove(const std::string& id) = 0;
    
    virtual std::optional<Transaction> findById(const std::string& id, const std::string& tenantId) = 0;
    virtual std::vector<Transaction> findByTenant(const std::string& tenantId) = 0;
    virtual std::vector<Transaction> findByAccount(const std::string& accountId, const std::string& tenantId) = 0;
    virtual std::vector<Transaction> findByCategory(const std::string& categoryId, const std::string& tenantId) = 0;
    virtual std::vector<Transaction> findByDateRange(
        const std::string& startDate,
        const std::string& endDate,
        const std::string& tenantId
    ) = 0;
    virtual std::vector<Transaction> findByStatus(const std::string& status, const std::string& tenantId) = 0;
    virtual std::vector<Transaction> findByReference(
        const std::string& referenceId,
        const std::string& referenceType,
        const std::string& tenantId
    ) = 0;
    
    // Reports
    virtual double getTotalByType(
        const std::string& type,
        const std::string& tenantId,
        const std::string& startDate,
        const std::string& endDate
    ) = 0;
    
    virtual double getTotalByCategory(
        const std::string& categoryId,
        const std::string& tenantId,
        const std::string& startDate,
        const std::string& endDate
    ) = 0;
    
    virtual std::map<std::string, double> getTotalsByCategory(
        const std::string& type,
        const std::string& tenantId,
        const std::string& startDate,
        const std::string& endDate
    ) = 0;
    
    virtual std::map<std::string, double> getCashFlow(
        const std::string& tenantId,
        const std::string& startDate,
        const std::string& endDate
    ) = 0;
};

} // namespace Finance

