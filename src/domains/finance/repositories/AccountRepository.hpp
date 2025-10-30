#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "../entities/Account.hpp"

namespace Finance {

class AccountRepository {
public:
    virtual ~AccountRepository() = default;

    virtual bool save(const Account& account) = 0;
    virtual bool update(const Account& account) = 0;
    virtual bool remove(const std::string& id) = 0;
    
    virtual std::optional<Account> findById(const std::string& id, const std::string& tenantId) = 0;
    virtual std::vector<Account> findByTenant(const std::string& tenantId) = 0;
    virtual std::vector<Account> findByWorkspace(const std::string& workspaceId, const std::string& tenantId) = 0;
    virtual std::vector<Account> findByCurrency(const std::string& currency, const std::string& tenantId) = 0;
    virtual std::vector<Account> findActive(const std::string& tenantId) = 0;
    
    virtual bool updateBalance(const std::string& id, double newBalance) = 0;
    virtual double getTotalBalance(const std::string& tenantId, const std::string& currency) = 0;
};

} // namespace Finance

