#pragma once

#include <string>
#include <map>
#include <mutex>
#include <chrono>
#include <vector>
#include <sstream>

namespace Core::Utils {

/**
 * @brief Collector de métricas para monitoramento (formato Prometheus)
 * 
 * Suporta 3 tipos de métricas:
 * - Counter: Valor que sempre cresce (ex: total_requests)
 * - Gauge: Valor que pode subir e descer (ex: active_connections)
 * - Histogram: Distribuição de valores (ex: request_duration)
 */
class MetricsCollector {
public:
    MetricsCollector();
    
    // ===== COUNTERS =====
    /**
     * @brief Incrementa um counter
     * @param name Nome da métrica
     * @param labels Labels opcionais (ex: {"method": "POST", "endpoint": "/api/login"})
     * @param value Valor a adicionar (padrão: 1)
     */
    void incrementCounter(
        const std::string& name,
        const std::map<std::string, std::string>& labels = {},
        double value = 1.0
    );
    
    // ===== GAUGES =====
    /**
     * @brief Define valor de um gauge
     * @param name Nome da métrica
     * @param value Valor atual
     * @param labels Labels opcionais
     */
    void setGauge(
        const std::string& name,
        double value,
        const std::map<std::string, std::string>& labels = {}
    );
    
    /**
     * @brief Incrementa um gauge
     */
    void incrementGauge(
        const std::string& name,
        double value = 1.0,
        const std::map<std::string, std::string>& labels = {}
    );
    
    /**
     * @brief Decrementa um gauge
     */
    void decrementGauge(
        const std::string& name,
        double value = 1.0,
        const std::map<std::string, std::string>& labels = {}
    );
    
    // ===== HISTOGRAMS =====
    /**
     * @brief Observa um valor em um histogram (para medir latências)
     * @param name Nome da métrica
     * @param value Valor observado (ex: tempo em segundos)
     * @param labels Labels opcionais
     */
    void observeHistogram(
        const std::string& name,
        double value,
        const std::map<std::string, std::string>& labels = {}
    );
    
    // ===== EXPORT =====
    /**
     * @brief Exporta todas as métricas no formato Prometheus
     * @return String formatada para scraping do Prometheus
     */
    std::string exportPrometheus();
    
    /**
     * @brief Limpa todas as métricas (útil para testes)
     */
    void reset();

private:
    std::mutex mutex_;
    
    // Estrutura para armazenar métricas
    struct MetricValue {
        double value;
        std::chrono::system_clock::time_point lastUpdated;
    };
    
    struct HistogramData {
        std::vector<double> values;
        double sum;
        size_t count;
    };
    
    // Storage: key = "metric_name{label1="value1",label2="value2"}"
    std::map<std::string, MetricValue> counters_;
    std::map<std::string, MetricValue> gauges_;
    std::map<std::string, HistogramData> histograms_;
    
    // Helpers
    std::string buildKey(const std::string& name, const std::map<std::string, std::string>& labels);
    std::string formatLabels(const std::map<std::string, std::string>& labels);
};

} // namespace Core::Utils

