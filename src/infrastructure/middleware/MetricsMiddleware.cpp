#include "MetricsMiddleware.hpp"
#include "../../core/utils/LoggerNew.hpp"
#include <chrono>

namespace Core::Http {

// Factory function implementation only - no class needed

MiddlewareFunction createMetricsMiddleware(std::shared_ptr<Core::Utils::MetricsCollector> collector) {
    // Capturar o collector diretamente no lambda para evitar lifetime issues
    return [collector](Request& req, Response& res, std::function<void()> next) {
        // Pular a rota /metrics para evitar recursão infinita
        if (req.getPath() == "/metrics") {
            next();
            return;
        }
        
        // Timestamp inicial
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Incrementar gauge de requisições ativas
        collector->incrementGauge("http_requests_in_flight");
        
        // Pegar método e path
        std::string method = req.methodToString();
        std::string path = req.getPath();
        
        // Executar próximo middleware/handler
        next();
        
        // Timestamp final
        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = endTime - startTime;
        double durationSeconds = duration.count();
        
        // Decrementar gauge de requisições ativas
        collector->decrementGauge("http_requests_in_flight");
        
        // Pegar status code
        int statusCode = static_cast<int>(res.getStatus());
        std::string statusStr = std::to_string(statusCode);
        
        // Labels para as métricas
        std::map<std::string, std::string> labels = {
            {"method", method},
            {"path", path},
            {"status", statusStr}
        };
        
        // Incrementar counter de requisições
        collector->incrementCounter("http_requests_total", labels);
        
        // Observar latência no histogram
        collector->observeHistogram("http_request_duration_seconds", durationSeconds, labels);
        
        // LOG_DEBUG("Metrics: " + method + " " + path + " " + statusStr + " (" + 
        //           std::to_string(durationSeconds * 1000.0) + "ms)");
    };
}

} // namespace Core::Http

