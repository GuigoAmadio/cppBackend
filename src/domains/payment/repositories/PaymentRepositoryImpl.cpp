#include "PaymentRepositoryImpl.hpp"
#include "../../../core/utils/Logger.hpp"
#include <sstream>
#include <algorithm>

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Payment {

// Helper function to escape SQL strings
std::string escapeSql(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        if (c == '\'') {
            escaped += "''";  // Escape single quote as ''
        } else {
            escaped += c;
        }
    }
    return escaped;
}

// Helper function to convert string to valid JSON
std::string toJsonString(const std::string& str) {
    // If already looks like JSON (starts with { or [), return as-is
    if (!str.empty() && (str[0] == '{' || str[0] == '[' || str[0] == '"')) {
        return str;
    }
    
    // Otherwise, wrap as JSON string
    std::string json = "\"";
    for (char c : str) {
        if (c == '"' || c == '\\') {
            json += '\\';
        }
        json += c;
    }
    json += "\"";
    return json;
}

PaymentRepositoryImpl::PaymentRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {
    Utils::Logger::info("[PaymentRepository] Initialized");
}

bool PaymentRepositoryImpl::save(const Payment& payment) {
    Utils::Logger::info("[PaymentRepository::save] Saving payment: " + payment.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::ostringstream query;
        query << "INSERT INTO payments (id, order_id, subscription_id, customer_id, tenant_id, amount, currency, "
              << "provider, status, provider_payment_id, provider_metadata, payment_method, "
              << "payment_details, paid_at, expires_at, created_by, updated_by, created_at, updated_at) "
              << "VALUES ("
              << "'" << payment.getId() << "', "
              << (payment.getOrderId() ? "'" + escapeSql(*payment.getOrderId()) + "'" : "NULL") << ", "
              << (payment.getSubscriptionId() ? "'" + escapeSql(*payment.getSubscriptionId()) + "'" : "NULL") << ", "
              << "'" << payment.getCustomerId() << "', "
              << "'" << payment.getTenantId() << "', "
              << payment.getAmount() << ", "
              << "'" << payment.getCurrency() << "', "
              << "'" << payment.getProvider().toString() << "', "
              << "'" << payment.getStatus().toString() << "', "
              << (payment.getProviderPaymentId() ? "'" + escapeSql(*payment.getProviderPaymentId()) + "'" : "NULL") << ", "
              << (payment.getProviderMetadata() ? "'" + escapeSql(toJsonString(*payment.getProviderMetadata())) + "'::jsonb" : "NULL") << ", "
              << (payment.getPaymentMethod() ? "'" + escapeSql(*payment.getPaymentMethod()) + "'" : "NULL") << ", "
              << (payment.getPaymentDetails() ? "'" + escapeSql(toJsonString(*payment.getPaymentDetails())) + "'::jsonb" : "NULL") << ", "
              << (payment.getPaidAt() ? "'" + *payment.getPaidAt() + "'" : "NULL") << ", "
              << (payment.getExpiresAt() ? "'" + *payment.getExpiresAt() + "'" : "NULL") << ", "
              << (payment.getCreatedBy() ? "'" + *payment.getCreatedBy() + "'" : "NULL") << ", "
              << (payment.getUpdatedBy() ? "'" + *payment.getUpdatedBy() + "'" : "NULL") << ", "
              << "CURRENT_TIMESTAMP, CURRENT_TIMESTAMP"
              << ") "
              << "ON CONFLICT (id) DO UPDATE SET "
              << "status = '" << payment.getStatus().toString() << "', "
              << "provider_payment_id = " << (payment.getProviderPaymentId() ? "'" + escapeSql(*payment.getProviderPaymentId()) + "'" : "NULL") << ", "
              << "provider_metadata = " << (payment.getProviderMetadata() ? "'" + escapeSql(toJsonString(*payment.getProviderMetadata())) + "'::jsonb" : "NULL") << ", "
              << "paid_at = " << (payment.getPaidAt() ? "'" + *payment.getPaidAt() + "'" : "NULL") << ", "
              << "updated_by = " << (payment.getUpdatedBy() ? "'" + *payment.getUpdatedBy() + "'" : "NULL") << ", "
              << "updated_at = CURRENT_TIMESTAMP";

        Utils::Logger::debug("[PaymentRepository::save] SQL: " + query.str());
        
        auto result = conn->execute(query.str());
        
        if (result.isSuccess()) {
            Utils::Logger::info("[PaymentRepository::save] Payment saved successfully!");
            return true;
        } else {
            Utils::Logger::error("[PaymentRepository::save] Failed to save payment - SQL ERROR!");
            Utils::Logger::error("[PaymentRepository::save] QUERY: " + query.str());
            return false;
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[PaymentRepository::save] Exception: " + std::string(e.what()));
        return false;
    }
}

std::optional<Payment> PaymentRepositoryImpl::findById(const std::string& id) {
    Utils::Logger::info("[PaymentRepository::findById] Finding payment: " + id);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM payments WHERE id = '" + id + "'";
        Utils::Logger::debug("[PaymentRepository::findById] SQL: " + query);
        
        auto result = conn->execute(query);
        
        if (result.isSuccess() && result.rowCount() > 0) {
            Utils::Logger::info("[PaymentRepository::findById] Payment found!");
            return mapToPayment(result, 0);
        }
        
        Utils::Logger::info("[PaymentRepository::findById] Payment not found");
        return std::nullopt;
    } catch (const std::exception& e) {
        Utils::Logger::error("[PaymentRepository::findById] Exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<Payment> PaymentRepositoryImpl::findByOrderId(const std::string& orderId) {
    Utils::Logger::info("[PaymentRepository::findByOrderId] Finding payments for order: " + orderId);
    
    std::vector<Payment> payments;
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM payments WHERE order_id = '" + orderId + "' ORDER BY created_at DESC";
        Utils::Logger::debug("[PaymentRepository::findByOrderId] SQL: " + query);
        
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            for (size_t i = 0; i < result.rowCount(); ++i) {
                payments.push_back(mapToPayment(result, i));
            }
            Utils::Logger::info("[PaymentRepository::findByOrderId] Found " + std::to_string(payments.size()) + " payment(s)");
        }
        
        return payments;
    } catch (const std::exception& e) {
        Utils::Logger::error("[PaymentRepository::findByOrderId] Exception: " + std::string(e.what()));
        return payments;
    }
}

std::vector<Payment> PaymentRepositoryImpl::findByCustomerId(const std::string& customerId) {
    Utils::Logger::info("[PaymentRepository::findByCustomerId] Finding payments for customer: " + customerId);
    
    std::vector<Payment> payments;
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM payments WHERE customer_id = '" + customerId + "' ORDER BY created_at DESC";
        Utils::Logger::debug("[PaymentRepository::findByCustomerId] SQL: " + query);
        
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            for (size_t i = 0; i < result.rowCount(); ++i) {
                payments.push_back(mapToPayment(result, i));
            }
            Utils::Logger::info("[PaymentRepository::findByCustomerId] Found " + std::to_string(payments.size()) + " payment(s)");
        }
        
        return payments;
    } catch (const std::exception& e) {
        Utils::Logger::error("[PaymentRepository::findByCustomerId] Exception: " + std::string(e.what()));
        return payments;
    }
}

std::vector<Payment> PaymentRepositoryImpl::findByTenant(const std::string& tenantId) {
    Utils::Logger::info("[PaymentRepository::findByTenant] Finding payments for tenant: " + tenantId);
    
    std::vector<Payment> payments;
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM payments WHERE tenant_id = '" + tenantId + "' ORDER BY created_at DESC LIMIT 100";
        Utils::Logger::debug("[PaymentRepository::findByTenant] SQL: " + query);
        
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            for (size_t i = 0; i < result.rowCount(); ++i) {
                payments.push_back(mapToPayment(result, i));
            }
            Utils::Logger::info("[PaymentRepository::findByTenant] Found " + std::to_string(payments.size()) + " payment(s)");
        }
        
        return payments;
    } catch (const std::exception& e) {
        Utils::Logger::error("[PaymentRepository::findByTenant] Exception: " + std::string(e.what()));
        return payments;
    }
}

bool PaymentRepositoryImpl::updateStatus(const std::string& id, const std::string& status) {
    Utils::Logger::info("[PaymentRepository::updateStatus] Updating payment " + id + " to status: " + status);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "UPDATE payments SET status = '" + status + "', updated_at = CURRENT_TIMESTAMP WHERE id = '" + id + "'";
        Utils::Logger::debug("[PaymentRepository::updateStatus] SQL: " + query);
        
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[PaymentRepository::updateStatus] Status updated successfully!");
            return true;
        } else {
            Utils::Logger::error("[PaymentRepository::updateStatus] Failed to update status");
            return false;
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[PaymentRepository::updateStatus] Exception: " + std::string(e.what()));
        return false;
    }
}

bool PaymentRepositoryImpl::remove(const std::string& id) {
    Utils::Logger::info("[PaymentRepository::remove] Removing payment: " + id);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "DELETE FROM payments WHERE id = '" + id + "'";
        Utils::Logger::debug("[PaymentRepository::remove] SQL: " + query);
        
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[PaymentRepository::remove] Payment removed successfully!");
            return true;
        } else {
            Utils::Logger::error("[PaymentRepository::remove] Failed to remove payment");
            return false;
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[PaymentRepository::remove] Exception: " + std::string(e.what()));
        return false;
    }
}

bool PaymentRepositoryImpl::saveTransaction(const Transaction& transaction) {
    Utils::Logger::info("[PaymentRepository::saveTransaction] Saving transaction: " + transaction.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::ostringstream query;
        query << "INSERT INTO payment_transactions (id, payment_id, tenant_id, transaction_type, "
              << "status, amount, currency, provider_transaction_id, provider_response, reason, notes, created_by) "
              << "VALUES ("
              << "'" << transaction.getId() << "', "
              << "'" << transaction.getPaymentId() << "', "
              << "'" << transaction.getTenantId() << "', "
              << "'" << transaction.getType().toString() << "', "
              << "'" << transaction.getStatus() << "', "
              << transaction.getAmount() << ", "
              << "'" << transaction.getCurrency() << "', "
              << (transaction.getProviderTransactionId() ? "'" + *transaction.getProviderTransactionId() + "'" : "NULL") << ", "
              << (transaction.getProviderResponse() ? "'" + *transaction.getProviderResponse() + "'" : "NULL") << ", "
              << (transaction.getReason() ? "'" + *transaction.getReason() + "'" : "NULL") << ", "
              << (transaction.getNotes() ? "'" + *transaction.getNotes() + "'" : "NULL") << ", "
              << (transaction.getCreatedBy() ? "'" + *transaction.getCreatedBy() + "'" : "NULL")
              << ")";

        Utils::Logger::debug("[PaymentRepository::saveTransaction] SQL: " + query.str());
        
        auto result = conn->execute(query.str());
        
        if (result.isSuccess()) {
            Utils::Logger::info("[PaymentRepository::saveTransaction] Transaction saved successfully!");
            return true;
        } else {
            Utils::Logger::error("[PaymentRepository::saveTransaction] Failed to save transaction");
            return false;
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[PaymentRepository::saveTransaction] Exception: " + std::string(e.what()));
        return false;
    }
}

std::vector<Transaction> PaymentRepositoryImpl::findTransactionsByPaymentId(const std::string& paymentId) {
    Utils::Logger::info("[PaymentRepository::findTransactionsByPaymentId] Finding transactions for payment: " + paymentId);
    
    std::vector<Transaction> transactions;
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM payment_transactions WHERE payment_id = '" + paymentId + "' ORDER BY created_at DESC";
        Utils::Logger::debug("[PaymentRepository::findTransactionsByPaymentId] SQL: " + query);
        
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            for (size_t i = 0; i < result.rowCount(); ++i) {
                transactions.push_back(mapToTransaction(result, i));
            }
            Utils::Logger::info("[PaymentRepository::findTransactionsByPaymentId] Found " + std::to_string(transactions.size()) + " transaction(s)");
        }
        
        return transactions;
    } catch (const std::exception& e) {
        Utils::Logger::error("[PaymentRepository::findTransactionsByPaymentId] Exception: " + std::string(e.what()));
        return transactions;
    }
}

bool PaymentRepositoryImpl::updateTransactionStatus(const std::string& id, const std::string& status) {
    Utils::Logger::info("[PaymentRepository::updateTransactionStatus] Updating transaction " + id + " to status: " + status);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "UPDATE payment_transactions SET status = '" + status + "' WHERE id = '" + id + "'";
        Utils::Logger::debug("[PaymentRepository::updateTransactionStatus] SQL: " + query);
        
        auto result = conn->execute(query);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[PaymentRepository::updateTransactionStatus] Status updated successfully!");
            return true;
        } else {
            Utils::Logger::error("[PaymentRepository::updateTransactionStatus] Failed to update status");
            return false;
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[PaymentRepository::updateTransactionStatus] Exception: " + std::string(e.what()));
        return false;
    }
}

Payment PaymentRepositoryImpl::mapToPayment(const Core::Database::QueryResult& result, size_t row) {
    // Colunas do SELECT * FROM payments (ordem física da tabela):
    // 0:id, 1:order_id, 2:customer_id, 3:tenant_id, 4:amount, 5:currency, 
    // 6:provider, 7:status, 8:provider_payment_id, 9:provider_metadata,
    // 10:payment_method, 11:payment_details, 12:paid_at, 13:expires_at,
    // 14:created_at, 15:updated_at, 16:created_by, 17:updated_by, 18:subscription_id
    
    auto id = result.getValue(row, 0);
    auto orderId = result.getValue(row, 1);
    auto customerId = result.getValue(row, 2);
    auto tenantId = result.getValue(row, 3);
    
    // Parse amount with validation
    auto amountStr = result.getValue(row, 4);
    double amount = 0.0;
    try {
        if (!amountStr.empty()) {
            amount = std::stod(amountStr);
        }
    } catch (const std::exception& e) {
        Utils::Logger::error("[PaymentRepository::mapToPayment] Invalid amount value: " + amountStr);
        throw std::runtime_error("Invalid amount in database: " + amountStr);
    }
    
    auto currency = result.getValue(row, 5);
    auto providerStr = result.getValue(row, 6);
    auto statusStr = result.getValue(row, 7);
    auto subscriptionId = result.getValue(row, 18);  // subscription_id está no final!

    Payment payment(
        id, customerId, tenantId, amount, currency,
        PaymentProvider(providerStr),
        PaymentStatus(statusStr)
    );

    // Set order_id and subscription_id (optional)
    if (!orderId.empty()) payment.setOrderId(orderId);
    if (!subscriptionId.empty()) payment.setSubscriptionId(subscriptionId);

    // Set optional fields
    auto providerPaymentId = result.getValue(row, 8);
    if (!providerPaymentId.empty()) payment.setProviderPaymentId(providerPaymentId);

    auto providerMetadata = result.getValue(row, 9);
    if (!providerMetadata.empty()) payment.setProviderMetadata(providerMetadata);

    auto paymentMethod = result.getValue(row, 10);
    if (!paymentMethod.empty()) payment.setPaymentMethod(paymentMethod);

    auto paymentDetails = result.getValue(row, 11);
    if (!paymentDetails.empty()) payment.setPaymentDetails(paymentDetails);

    auto paidAt = result.getValue(row, 12);
    if (!paidAt.empty()) payment.setPaidAt(paidAt);

    auto expiresAt = result.getValue(row, 13);
    if (!expiresAt.empty()) payment.setExpiresAt(expiresAt);

    auto createdAt = result.getValue(row, 14);
    if (!createdAt.empty()) payment.setCreatedAt(createdAt);

    auto updatedAt = result.getValue(row, 15);
    if (!updatedAt.empty()) payment.setUpdatedAt(updatedAt);

    auto createdBy = result.getValue(row, 16);
    if (!createdBy.empty()) payment.setCreatedBy(createdBy);

    auto updatedBy = result.getValue(row, 17);
    if (!updatedBy.empty()) payment.setUpdatedBy(updatedBy);

    return payment;
}

Transaction PaymentRepositoryImpl::mapToTransaction(const Core::Database::QueryResult& result, size_t row) {
    // Colunas do SELECT * FROM payment_transactions:
    // 0:id, 1:payment_id, 2:tenant_id, 3:transaction_type, 4:status, 
    // 5:amount, 6:currency, 7:provider_transaction_id, 8:provider_response,
    // 9:reason, 10:notes, 11:created_at, 12:created_by
    
    auto id = result.getValue(row, 0);
    auto paymentId = result.getValue(row, 1);
    auto tenantId = result.getValue(row, 2);
    auto typeStr = result.getValue(row, 3);
    auto status = result.getValue(row, 4);
    double amount = std::stod(result.getValue(row, 5));
    auto currency = result.getValue(row, 6);

    Transaction transaction(
        id, paymentId, tenantId,
        TransactionType(typeStr),
        status, amount, currency
    );

    // Set optional fields
    auto providerTransactionId = result.getValue(row, 7);
    if (!providerTransactionId.empty()) transaction.setProviderTransactionId(providerTransactionId);

    auto providerResponse = result.getValue(row, 8);
    if (!providerResponse.empty()) transaction.setProviderResponse(providerResponse);

    auto reason = result.getValue(row, 9);
    if (!reason.empty()) transaction.setReason(reason);

    auto notes = result.getValue(row, 10);
    if (!notes.empty()) transaction.setNotes(notes);

    auto createdAt = result.getValue(row, 11);
    if (!createdAt.empty()) transaction.setCreatedAt(createdAt);

    auto createdBy = result.getValue(row, 12);
    if (!createdBy.empty()) transaction.setCreatedBy(createdBy);

    return transaction;
}

} // namespace Domain::Payment

