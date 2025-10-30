#pragma once

#include "../../core/http/Middleware.hpp"
#include "../../core/utils/MetricsCollector.hpp"
#include <memory>

namespace Core::Http {

/**
 * @brief Factory function para criar MetricsMiddleware
 * 
 * Coleta automaticamente:
 * - http_requests_total (counter) - Total de requisições
 * - http_request_duration_seconds (histogram) - Latência das requisições
 * - http_requests_in_flight (gauge) - Requisições ativas no momento
 * 
 * Todas as métricas incluem labels: method, path, status
 * 
 * @param collector Shared pointer para o MetricsCollector global
 * @return MiddlewareFunction que coleta métricas
 */
MiddlewareFunction createMetricsMiddleware(std::shared_ptr<Core::Utils::MetricsCollector> collector);

} // namespace Core::Http

