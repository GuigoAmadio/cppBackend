#pragma once
#include <memory>
#include <map>
#include "../TransactionRepository.hpp"
#include "../../../../core/database/ConnectionPool.hpp"

namespace Finance {

class TransactionRepositoryImpl : public TransactionRepository {
public:
    explicit TransactionRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);

    bool save(const Transaction& transaction) override;
    bool update(const Transaction& transaction) override;
    bool remove(const std::string& id) override;
    
    std::optional<Transaction> findById(const std::string& id, const std::string& tenantId) override;
    std::vector<Transaction> findByTenant(const std::string& tenantId) override;
    std::vector<Transaction> findByAccount(const std::string& accountId, const std::string& tenantId) override;
    std::vector<Transaction> findByCategory(const std::string& categoryId, const std::string& tenantId) override;
    std::vector<Transaction> findByDateRange(const std::string& startDate, const std::string& endDate, const std::string& tenantId) override;
    std::vector<Transaction> findByStatus(const std::string& status, const std::string& tenantId) override;
    std::vector<Transaction> findByReference(const std::string& referenceId, const std::string& referenceType, const std::string& tenantId) override;
    
    double getTotalByType(const std::string& type, const std::string& tenantId, const std::string& startDate, const std::string& endDate) override;
    double getTotalByCategory(const std::string& categoryId, const std::string& tenantId, const std::string& startDate, const std::string& endDate) override;
    std::map<std::string, double> getTotalsByCategory(const std::string& type, const std::string& tenantId, const std::string& startDate, const std::string& endDate) override;
    std::map<std::string, double> getCashFlow(const std::string& tenantId, const std::string& startDate, const std::string& endDate) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    Transaction mapToTransaction(const Core::Database::QueryResult& result, int row);
};

} // namespace Finance

