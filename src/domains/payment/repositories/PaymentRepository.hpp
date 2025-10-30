#pragma once

#include <memory>
#include <optional>
#include <vector>
#include "../entities/Payment.hpp"
#include "../entities/Transaction.hpp"

namespace Domain::Payment {

/**
 * @brief Interface do repositório de Payment
 */
class PaymentRepository {
public:
    virtual ~PaymentRepository() = default;

    // Payment operations
    virtual bool save(const Payment& payment) = 0;
    virtual std::optional<Payment> findById(const std::string& id) = 0;
    virtual std::vector<Payment> findByOrderId(const std::string& orderId) = 0;
    virtual std::vector<Payment> findByCustomerId(const std::string& customerId) = 0;
    virtual std::vector<Payment> findByTenant(const std::string& tenantId) = 0;
    virtual bool updateStatus(const std::string& id, const std::string& status) = 0;
    virtual bool remove(const std::string& id) = 0;

    // Transaction operations
    virtual bool saveTransaction(const Transaction& transaction) = 0;
    virtual std::vector<Transaction> findTransactionsByPaymentId(const std::string& paymentId) = 0;
    virtual bool updateTransactionStatus(const std::string& id, const std::string& status) = 0;
};

} // namespace Domain::Payment

