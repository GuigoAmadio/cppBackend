#include "InvoiceRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"
#include <sstream>

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Invoicing {

std::string InvoiceRepositoryImpl::escapeSql(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        if (c == '\'') {
            escaped += "''";
        } else {
            escaped += c;
        }
    }
    return escaped;
}

InvoiceRepositoryImpl::InvoiceRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {
    Utils::Logger::info("[InvoiceRepository] Initialized");
}

bool InvoiceRepositoryImpl::save(const Invoice& invoice) {
    Utils::Logger::info("[InvoiceRepository::save] Saving invoice: " + invoice.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::ostringstream query;
        query << "INSERT INTO invoices (id, invoice_number, tenant_id, customer_id, order_id, subscription_id, "
              << "subtotal, tax, discount, total, amount_paid, amount_due, currency, status, invoice_type, "
              << "issue_date, due_date, paid_at, description, notes, terms, created_by, updated_by) "
              << "VALUES ("
              << "'" << invoice.getId() << "', "
              << "'" << escapeSql(invoice.getInvoiceNumber()) << "', "
              << "'" << escapeSql(invoice.getTenantId()) << "', "
              << "'" << escapeSql(invoice.getCustomerId()) << "', "
              << (invoice.getOrderId() ? "'" + escapeSql(*invoice.getOrderId()) + "'" : "NULL") << ", "
              << (invoice.getSubscriptionId() ? "'" + escapeSql(*invoice.getSubscriptionId()) + "'" : "NULL") << ", "
              << invoice.getSubtotal() << ", "
              << invoice.getTax() << ", "
              << invoice.getDiscount() << ", "
              << invoice.getTotal() << ", "
              << invoice.getAmountPaid() << ", "
              << invoice.getAmountDue() << ", "
              << "'" << escapeSql(invoice.getCurrency()) << "', "
              << "'" << invoice.getStatus().toString() << "', "
              << "'" << invoice.getType().toString() << "', "
              << "'" << invoice.getIssueDate() << "', "
              << (invoice.getDueDate() ? "'" + *invoice.getDueDate() + "'" : "NULL") << ", "
              << (invoice.getPaidAt() ? "'" + *invoice.getPaidAt() + "'" : "NULL") << ", "
              << (invoice.getDescription() ? "'" + escapeSql(*invoice.getDescription()) + "'" : "NULL") << ", "
              << (invoice.getNotes() ? "'" + escapeSql(*invoice.getNotes()) + "'" : "NULL") << ", "
              << (invoice.getTerms() ? "'" + escapeSql(*invoice.getTerms()) + "'" : "NULL") << ", "
              << (invoice.getCreatedBy() ? "'" + *invoice.getCreatedBy() + "'" : "NULL") << ", "
              << (invoice.getUpdatedBy() ? "'" + *invoice.getUpdatedBy() + "'" : "NULL")
              << ") ON CONFLICT (id) DO UPDATE SET "
              << "subtotal = " << invoice.getSubtotal() << ", "
              << "tax = " << invoice.getTax() << ", "
              << "discount = " << invoice.getDiscount() << ", "
              << "total = " << invoice.getTotal() << ", "
              << "amount_paid = " << invoice.getAmountPaid() << ", "
              << "amount_due = " << invoice.getAmountDue() << ", "
              << "status = '" << invoice.getStatus().toString() << "', "
              << "paid_at = " << (invoice.getPaidAt() ? "'" + *invoice.getPaidAt() + "'" : "NULL") << ", "
              << "notes = " << (invoice.getNotes() ? "'" + escapeSql(*invoice.getNotes()) + "'" : "NULL") << ", "
              << "updated_by = " << (invoice.getUpdatedBy() ? "'" + *invoice.getUpdatedBy() + "'" : "NULL") << ", "
              << "updated_at = CURRENT_TIMESTAMP";
        
        auto result = conn->execute(query.str());
        
        if (result.isSuccess()) {
            Utils::Logger::info("[InvoiceRepository::save] Invoice saved successfully!");
            return true;
        }
        
        Utils::Logger::error("[InvoiceRepository::save] Failed to save invoice");
        return false;
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::save] Exception: " + std::string(e.what()));
        return false;
    }
}

std::optional<Invoice> InvoiceRepositoryImpl::findById(const std::string& id) {
    Utils::Logger::info("[InvoiceRepository::findById] Finding invoice: " + id);
    
    try {
        auto conn = pool_->acquire();
        std::string query = "SELECT * FROM invoices WHERE id = '" + escapeSql(id) + "'";
        auto result = conn->execute(query);
        
        if (result.isSuccess() && result.rowCount() > 0) {
            return mapToInvoice(result, 0);
        }
        
        return std::nullopt;
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::findById] Exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<Invoice> InvoiceRepositoryImpl::findByTenant(const std::string& tenantId) {
    Utils::Logger::info("[InvoiceRepository::findByTenant] Finding invoices for tenant: " + tenantId);
    
    std::vector<Invoice> invoices;
    try {
        auto conn = pool_->acquire();
        std::string query = "SELECT * FROM invoices WHERE tenant_id = '" + escapeSql(tenantId) + "' ORDER BY created_at DESC";
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            for (size_t i = 0; i < result.rowCount(); i++) {
                invoices.push_back(mapToInvoice(result, i));
            }
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::findByTenant] Exception: " + std::string(e.what()));
    }
    
    return invoices;
}

std::vector<Invoice> InvoiceRepositoryImpl::findByCustomer(const std::string& customerId) {
    std::vector<Invoice> invoices;
    try {
        auto conn = pool_->acquire();
        std::string query = "SELECT * FROM invoices WHERE customer_id = '" + escapeSql(customerId) + "' ORDER BY created_at DESC";
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            for (size_t i = 0; i < result.rowCount(); i++) {
                invoices.push_back(mapToInvoice(result, i));
            }
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::findByCustomer] Exception: " + std::string(e.what()));
    }
    
    return invoices;
}

std::vector<Invoice> InvoiceRepositoryImpl::findByOrder(const std::string& orderId) {
    std::vector<Invoice> invoices;
    try {
        auto conn = pool_->acquire();
        std::string query = "SELECT * FROM invoices WHERE order_id = '" + escapeSql(orderId) + "'";
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            for (size_t i = 0; i < result.rowCount(); i++) {
                invoices.push_back(mapToInvoice(result, i));
            }
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::findByOrder] Exception: " + std::string(e.what()));
    }
    
    return invoices;
}

std::vector<Invoice> InvoiceRepositoryImpl::findBySubscription(const std::string& subscriptionId) {
    std::vector<Invoice> invoices;
    try {
        auto conn = pool_->acquire();
        std::string query = "SELECT * FROM invoices WHERE subscription_id = '" + escapeSql(subscriptionId) + "'";
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            for (size_t i = 0; i < result.rowCount(); i++) {
                invoices.push_back(mapToInvoice(result, i));
            }
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::findBySubscription] Exception: " + std::string(e.what()));
    }
    
    return invoices;
}

std::vector<Invoice> InvoiceRepositoryImpl::findByStatus(const std::string& tenantId, const std::string& status) {
    std::vector<Invoice> invoices;
    try {
        auto conn = pool_->acquire();
        std::string query = "SELECT * FROM invoices WHERE tenant_id = '" + escapeSql(tenantId) + 
                          "' AND status = '" + escapeSql(status) + "' ORDER BY created_at DESC";
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            for (size_t i = 0; i < result.rowCount(); i++) {
                invoices.push_back(mapToInvoice(result, i));
            }
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::findByStatus] Exception: " + std::string(e.what()));
    }
    
    return invoices;
}

bool InvoiceRepositoryImpl::remove(const std::string& id) {
    Utils::Logger::info("[InvoiceRepository::remove] Removing invoice: " + id);
    
    try {
        auto conn = pool_->acquire();
        std::string query = "DELETE FROM invoices WHERE id = '" + escapeSql(id) + "'";
        auto result = conn->execute(query);
        
        return result.isSuccess();
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::remove] Exception: " + std::string(e.what()));
        return false;
    }
}

bool InvoiceRepositoryImpl::saveItem(const InvoiceItem& item) {
    Utils::Logger::info("[InvoiceRepository::saveItem] Saving invoice item: " + item.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::ostringstream query;
        query << "INSERT INTO invoice_items (id, invoice_id, tenant_id, product_id, description, "
              << "quantity, unit_price, discount, tax_rate, tax_amount, total, sort_order) "
              << "VALUES ("
              << "'" << item.getId() << "', "
              << "'" << escapeSql(item.getInvoiceId()) << "', "
              << "'" << escapeSql(item.getTenantId()) << "', "
              << (item.getProductId() ? "'" + escapeSql(*item.getProductId()) + "'" : "NULL") << ", "
              << "'" << escapeSql(item.getDescription()) << "', "
              << item.getQuantity() << ", "
              << item.getUnitPrice() << ", "
              << item.getDiscount() << ", "
              << item.getTaxRate() << ", "
              << item.getTaxAmount() << ", "
              << item.getTotal() << ", "
              << item.getSortOrder()
              << ") ON CONFLICT (id) DO UPDATE SET "
              << "description = '" << escapeSql(item.getDescription()) << "', "
              << "quantity = " << item.getQuantity() << ", "
              << "unit_price = " << item.getUnitPrice() << ", "
              << "discount = " << item.getDiscount() << ", "
              << "tax_rate = " << item.getTaxRate() << ", "
              << "tax_amount = " << item.getTaxAmount() << ", "
              << "total = " << item.getTotal();
        
        auto result = conn->execute(query.str());
        return result.isSuccess();
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::saveItem] Exception: " + std::string(e.what()));
        return false;
    }
}

std::vector<InvoiceItem> InvoiceRepositoryImpl::findItemsByInvoice(const std::string& invoiceId) {
    std::vector<InvoiceItem> items;
    
    try {
        auto conn = pool_->acquire();
        std::string query = "SELECT * FROM invoice_items WHERE invoice_id = '" + escapeSql(invoiceId) + "' ORDER BY sort_order";
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            for (size_t i = 0; i < result.rowCount(); i++) {
                items.push_back(mapToInvoiceItem(result, i));
            }
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::findItemsByInvoice] Exception: " + std::string(e.what()));
    }
    
    return items;
}

bool InvoiceRepositoryImpl::removeItem(const std::string& itemId) {
    try {
        auto conn = pool_->acquire();
        std::string query = "DELETE FROM invoice_items WHERE id = '" + escapeSql(itemId) + "'";
        auto result = conn->execute(query);
        return result.isSuccess();
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::removeItem] Exception: " + std::string(e.what()));
        return false;
    }
}

bool InvoiceRepositoryImpl::removeAllItems(const std::string& invoiceId) {
    try {
        auto conn = pool_->acquire();
        std::string query = "DELETE FROM invoice_items WHERE invoice_id = '" + escapeSql(invoiceId) + "'";
        auto result = conn->execute(query);
        return result.isSuccess();
    } catch (const std::exception& e) {
        Utils::Logger::error("[InvoiceRepository::removeAllItems] Exception: " + std::string(e.what()));
        return false;
    }
}

Invoice InvoiceRepositoryImpl::mapToInvoice(const Core::Database::QueryResult& result, size_t row) {
    // Ordem física das colunas da tabela invoices
    auto id = result.getValue(row, 0);
    auto invoiceNumber = result.getValue(row, 1);
    auto tenantId = result.getValue(row, 2);
    auto customerId = result.getValue(row, 3);
    auto orderId = result.getValue(row, 4);
    auto subscriptionId = result.getValue(row, 5);
    
    double subtotal = std::stod(result.getValue(row, 6));
    double tax = std::stod(result.getValue(row, 7));
    double discount = std::stod(result.getValue(row, 8));
    double total = std::stod(result.getValue(row, 9));
    double amountPaid = std::stod(result.getValue(row, 10));
    double amountDue = std::stod(result.getValue(row, 11));
    
    auto currency = result.getValue(row, 12);
    auto statusStr = result.getValue(row, 13);
    auto typeStr = result.getValue(row, 14);
    
    Invoice invoice(id, invoiceNumber, tenantId, customerId,
                   InvoiceStatus(statusStr), InvoiceType(typeStr));
    
    if (!orderId.empty()) invoice.setOrderId(orderId);
    if (!subscriptionId.empty()) invoice.setSubscriptionId(subscriptionId);
    
    invoice.setSubtotal(subtotal);
    invoice.setTax(tax);
    invoice.setDiscount(discount);
    invoice.setTotal(total);
    invoice.setAmountPaid(amountPaid);
    invoice.setCurrency(currency);
    
    auto issueDate = result.getValue(row, 15);
    if (!issueDate.empty()) invoice.setIssueDate(issueDate);
    
    auto dueDate = result.getValue(row, 16);
    if (!dueDate.empty()) invoice.setDueDate(dueDate);
    
    auto paidAt = result.getValue(row, 17);
    if (!paidAt.empty()) invoice.setPaidAt(paidAt);
    
    auto description = result.getValue(row, 18);
    if (!description.empty()) invoice.setDescription(description);
    
    auto notes = result.getValue(row, 19);
    if (!notes.empty()) invoice.setNotes(notes);
    
    auto terms = result.getValue(row, 20);
    if (!terms.empty()) invoice.setTerms(terms);
    
    return invoice;
}

InvoiceItem InvoiceRepositoryImpl::mapToInvoiceItem(const Core::Database::QueryResult& result, size_t row) {
    auto id = result.getValue(row, 0);
    auto invoiceId = result.getValue(row, 1);
    auto tenantId = result.getValue(row, 2);
    auto productId = result.getValue(row, 3);
    auto description = result.getValue(row, 4);
    double quantity = std::stod(result.getValue(row, 5));
    double unitPrice = std::stod(result.getValue(row, 6));
    
    InvoiceItem item(id, invoiceId, tenantId, description, quantity, unitPrice);
    
    if (!productId.empty()) item.setProductId(productId);
    
    double discount = std::stod(result.getValue(row, 7));
    double taxRate = std::stod(result.getValue(row, 8));
    int sortOrder = std::stoi(result.getValue(row, 11));
    
    item.setDiscount(discount);
    item.setTaxRate(taxRate);
    item.setSortOrder(sortOrder);
    
    return item;
}

} // namespace Domain::Invoicing

