#pragma once

#include <memory>
#include <optional>
#include <vector>
#include "../entities/Invoice.hpp"
#include "../entities/InvoiceItem.hpp"

namespace Domain::Invoicing {

/**
 * @brief Interface do repositório de invoices
 */
class InvoiceRepository {
public:
    virtual ~InvoiceRepository() = default;

    // Invoice operations
    virtual bool save(const Invoice& invoice) = 0;
    virtual std::optional<Invoice> findById(const std::string& id) = 0;
    virtual std::vector<Invoice> findByTenant(const std::string& tenantId) = 0;
    virtual std::vector<Invoice> findByCustomer(const std::string& customerId) = 0;
    virtual std::vector<Invoice> findByOrder(const std::string& orderId) = 0;
    virtual std::vector<Invoice> findBySubscription(const std::string& subscriptionId) = 0;
    virtual std::vector<Invoice> findByStatus(const std::string& tenantId, const std::string& status) = 0;
    virtual bool remove(const std::string& id) = 0;

    // Invoice Item operations
    virtual bool saveItem(const InvoiceItem& item) = 0;
    virtual std::vector<InvoiceItem> findItemsByInvoice(const std::string& invoiceId) = 0;
    virtual bool removeItem(const std::string& itemId) = 0;
    virtual bool removeAllItems(const std::string& invoiceId) = 0;
};

} // namespace Domain::Invoicing

