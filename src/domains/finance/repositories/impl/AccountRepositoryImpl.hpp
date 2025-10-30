#pragma once
#include <memory>
#include "../AccountRepository.hpp"
#include "../../../../core/database/ConnectionPool.hpp"

namespace Finance {

class AccountRepositoryImpl : public AccountRepository {
public:
    explicit AccountRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);

    bool save(const Account& account) override;
    bool update(const Account& account) override;
    bool remove(const std::string& id) override;
    
    std::optional<Account> findById(const std::string& id, const std::string& tenantId) override;
    std::vector<Account> findByTenant(const std::string& tenantId) override;
    std::vector<Account> findByWorkspace(const std::string& workspaceId, const std::string& tenantId) override;
    std::vector<Account> findByCurrency(const std::string& currency, const std::string& tenantId) override;
    std::vector<Account> findActive(const std::string& tenantId) override;
    
    bool updateBalance(const std::string& id, double newBalance) override;
    double getTotalBalance(const std::string& tenantId, const std::string& currency) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    
    Account mapToAccount(const Core::Database::QueryResult& result, int row);
};

} // namespace Finance

