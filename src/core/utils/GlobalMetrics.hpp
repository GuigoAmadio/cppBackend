#pragma once

#include <string>

namespace GlobalMetrics {

/**
 * @brief Registra a criação de um pedido
 * @param tenantId ID do tenant
 * @param totalAmount Valor total do pedido
 */
void recordOrderCreated(const std::string& tenantId, double totalAmount);

/**
 * @brief Registra o processamento de um pagamento
 * @param provider Provider do pagamento (stripe, paypal, pix, etc)
 * @param status Status do pagamento (completed, failed, etc)
 * @param amount Valor do pagamento
 * @param currency Moeda (BRL, USD, etc)
 */
void recordPaymentProcessed(const std::string& provider, const std::string& status,
                            double amount, const std::string& currency);

/**
 * @brief Registra a criação de uma assinatura
 * @param planId ID do plano
 * @param tenantId ID do tenant
 */
void recordSubscriptionCreated(const std::string& planId, const std::string& tenantId);

/**
 * @brief Registra envio de email
 * @param type Tipo do email (verification, reset_password, notification, etc)
 */
void recordEmailSent(const std::string& type);

/**
 * @brief Registra conclusão de uma task
 * @param userId ID do usuário que completou
 */
void recordTaskCompleted(const std::string& userId);

} // namespace GlobalMetrics


