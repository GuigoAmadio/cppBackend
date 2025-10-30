#pragma once

#include <memory>
#include "../InvoiceRepository.hpp"
#include "../../../../core/database/ConnectionPool.hpp"

namespace Domain::Invoicing {

class InvoiceRepositoryImpl : public InvoiceRepository {
public:
    explicit InvoiceRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);

    bool save(const Invoice& invoice) override;
    std::optional<Invoice> findById(const std::string& id) override;
    std::vector<Invoice> findByTenant(const std::string& tenantId) override;
    std::vector<Invoice> findByCustomer(const std::string& customerId) override;
    std::vector<Invoice> findByOrder(const std::string& orderId) override;
    std::vector<Invoice> findBySubscription(const std::string& subscriptionId) override;
    std::vector<Invoice> findByStatus(const std::string& tenantId, const std::string& status) override;
    bool remove(const std::string& id) override;

    bool saveItem(const InvoiceItem& item) override;
    std::vector<InvoiceItem> findItemsByInvoice(const std::string& invoiceId) override;
    bool removeItem(const std::string& itemId) override;
    bool removeAllItems(const std::string& invoiceId) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;

    Invoice mapToInvoice(const Core::Database::QueryResult& result, size_t row);
    InvoiceItem mapToInvoiceItem(const Core::Database::QueryResult& result, size_t row);
    std::string escapeSql(const std::string& str);
};

} // namespace Domain::Invoicing

