#pragma once

#include "PaymentRepository.hpp"
#include "../../../core/database/ConnectionPool.hpp"
#include <memory>

namespace Domain::Payment {

class PaymentRepositoryImpl : public PaymentRepository {
public:
    explicit PaymentRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);

    bool save(const Payment& payment) override;
    std::optional<Payment> findById(const std::string& id) override;
    std::vector<Payment> findByOrderId(const std::string& orderId) override;
    std::vector<Payment> findByCustomerId(const std::string& customerId) override;
    std::vector<Payment> findByTenant(const std::string& tenantId) override;
    bool updateStatus(const std::string& id, const std::string& status) override;
    bool remove(const std::string& id) override;

    bool saveTransaction(const Transaction& transaction) override;
    std::vector<Transaction> findTransactionsByPaymentId(const std::string& paymentId) override;
    bool updateTransactionStatus(const std::string& id, const std::string& status) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;

    Payment mapToPayment(const Core::Database::QueryResult& result, size_t row);
    Transaction mapToTransaction(const Core::Database::QueryResult& result, size_t row);
};

} // namespace Domain::Payment

