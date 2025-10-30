#pragma once
#include <string>
#include <chrono>
#include <atomic>
#include <sstream>
#include <iomanip>

namespace Core {
namespace Utils {

class IdGenerator {
public:
    /**
     * Gera um ID único usando:
     * - Timestamp em milissegundos
     * - Contador atômico (sequencial)
     * 
     * Formato: {prefix}-{timestamp_ms}-{counter}
     * Exemplo: acc-1761272196789-1
     */
    static std::string generate(const std::string& prefix = "id") {
        // Timestamp em milissegundos
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        
        // Contador atômico para garantir unicidade
        static std::atomic<uint32_t> counter{0};
        uint32_t count = counter.fetch_add(1, std::memory_order_relaxed);
        
        // Formato: prefix-timestamp_ms-counter
        std::ostringstream oss;
        oss << prefix << "-" << ms << "-" << count;
        return oss.str();
    }
    
    /**
     * Gera ID com formato curto (apenas últimos 6 dígitos do timestamp + contador)
     * Útil para IDs mais legíveis
     */
    static std::string generateShort(const std::string& prefix = "id") {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        
        static std::atomic<uint32_t> counter{0};
        uint32_t count = counter.fetch_add(1, std::memory_order_relaxed);
        
        // Usa apenas os últimos 6 dígitos do timestamp
        uint64_t short_ts = ms % 1000000;
        
        std::ostringstream oss;
        oss << prefix << "-" << short_ts << "-" << count;
        return oss.str();
    }
};

} // namespace Utils
} // namespace Core

