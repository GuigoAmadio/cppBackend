#include "MetricsCollector.hpp"
#include "LoggerNew.hpp"
#include <algorithm>
#include <cmath>

namespace Core::Utils {

MetricsCollector::MetricsCollector() {
    LOG_INFO("MetricsCollector initialized");
}

std::string MetricsCollector::buildKey(const std::string& name, const std::map<std::string, std::string>& labels) {
    if (labels.empty()) {
        return name;
    }
    return name + formatLabels(labels);
}

std::string MetricsCollector::formatLabels(const std::map<std::string, std::string>& labels) {
    if (labels.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto& [key, value] : labels) {
        if (!first) oss << ",";
        oss << key << "=\"" << value << "\"";
        first = false;
    }
    oss << "}";
    return oss.str();
}

void MetricsCollector::incrementCounter(
    const std::string& name,
    const std::map<std::string, std::string>& labels,
    double value
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string key = buildKey(name, labels);
    counters_[key].value += value;
    counters_[key].lastUpdated = std::chrono::system_clock::now();
}

void MetricsCollector::setGauge(
    const std::string& name,
    double value,
    const std::map<std::string, std::string>& labels
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string key = buildKey(name, labels);
    gauges_[key].value = value;
    gauges_[key].lastUpdated = std::chrono::system_clock::now();
}

void MetricsCollector::incrementGauge(
    const std::string& name,
    double value,
    const std::map<std::string, std::string>& labels
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string key = buildKey(name, labels);
    gauges_[key].value += value;
    gauges_[key].lastUpdated = std::chrono::system_clock::now();
}

void MetricsCollector::decrementGauge(
    const std::string& name,
    double value,
    const std::map<std::string, std::string>& labels
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string key = buildKey(name, labels);
    gauges_[key].value -= value;
    gauges_[key].lastUpdated = std::chrono::system_clock::now();
}

void MetricsCollector::observeHistogram(
    const std::string& name,
    double value,
    const std::map<std::string, std::string>& labels
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string key = buildKey(name, labels);
    
    if (histograms_.find(key) == histograms_.end()) {
        histograms_[key] = HistogramData{{}, 0.0, 0};
    }
    
    histograms_[key].values.push_back(value);
    histograms_[key].sum += value;
    histograms_[key].count++;
}

std::string MetricsCollector::exportPrometheus() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ostringstream output;
    
    // ===== COUNTERS =====
    if (!counters_.empty()) {
        output << "# TYPE http_requests_total counter\n";
        for (const auto& [key, data] : counters_) {
            output << key << " " << data.value << "\n";
        }
        output << "\n";
    }
    
    // ===== GAUGES =====
    if (!gauges_.empty()) {
        for (const auto& [key, data] : gauges_) {
            // Extrair nome da métrica (antes de '{')
            size_t pos = key.find('{');
            std::string metricName = (pos != std::string::npos) ? key.substr(0, pos) : key;
            
            output << "# TYPE " << metricName << " gauge\n";
            output << key << " " << data.value << "\n";
        }
        output << "\n";
    }
    
    // ===== HISTOGRAMS =====
    if (!histograms_.empty()) {
        for (const auto& [key, data] : histograms_) {
            // Extrair nome da métrica
            size_t pos = key.find('{');
            std::string metricName = (pos != std::string::npos) ? key.substr(0, pos) : key;
            std::string labels = (pos != std::string::npos) ? key.substr(pos) : "";
            
            output << "# TYPE " << metricName << " histogram\n";
            
            // Buckets: 0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0
            std::vector<double> buckets = {0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0};
            
            for (double bucket : buckets) {
                size_t count = 0;
                for (double val : data.values) {
                    if (val <= bucket) {
                        count++;
                    }
                }
                
                if (!labels.empty()) {
                    // Inserir le no labels existente
                    std::string modifiedLabels = labels;
                    modifiedLabels.insert(modifiedLabels.length() - 1, ",le=\"" + std::to_string(bucket) + "\"");
                    output << metricName << "_bucket" << modifiedLabels << " " << count << "\n";
                } else {
                    output << metricName << "_bucket{le=\"" << bucket << "\"} " << count << "\n";
                }
            }
            
            // +Inf bucket
            if (!labels.empty()) {
                std::string modifiedLabels = labels;
                modifiedLabels.insert(modifiedLabels.length() - 1, ",le=\"+Inf\"");
                output << metricName << "_bucket" << modifiedLabels << " " << data.count << "\n";
            } else {
                output << metricName << "_bucket{le=\"+Inf\"} " << data.count << "\n";
            }
            
            // Sum e count
            output << metricName << "_sum" << labels << " " << data.sum << "\n";
            output << metricName << "_count" << labels << " " << data.count << "\n";
        }
        output << "\n";
    }
    
    return output.str();
}

void MetricsCollector::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    counters_.clear();
    gauges_.clear();
    histograms_.clear();
    
    LOG_DEBUG("Metrics reset");
}

} // namespace Core::Utils

