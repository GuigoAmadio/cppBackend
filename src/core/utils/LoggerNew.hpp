#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <memory>

namespace Core::Utils {

/**
 * @brief Logger avançado com cores, níveis, file output e debug mode.
 * 
 * Features:
 * - Múltiplos níveis (TRACE, DEBUG, INFO, WARNING, ERROR, FATAL)
 * - Cores no terminal (Windows/Linux)
 * - Output para arquivo
 * - Thread-safe
 * - Debug mode (mostra arquivo e linha)
 * - Formatação customizável
 * 
 * @example
 *   Logger::init("app.log", Logger::Level::DEBUG);
 *   Logger::info("Servidor iniciado");
 *   Logger::debug("Debug info", __FILE__, __LINE__);
 *   Logger::error("Erro fatal!");
 */
class LoggerNew {
public:
    enum class Level {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARNING = 3,
        ERR = 4,      // Renomeado de ERROR para evitar conflito com macro Windows
        FATAL = 5,
        OFF = 99
    };
    
    /**
     * @brief Inicializa o logger.
     * 
     * @param logFile Path para arquivo de log (vazio = apenas console)
     * @param minLevel Nível mínimo para log
     * @param useColors Usar cores no terminal
     * @param debugMode Mostrar arquivo e linha no log
     */
    static void init(
        const std::string& logFile = "",
        Level minLevel = Level::INFO,
        bool useColors = true,
        bool debugMode = false
    );
    
    /**
     * @brief Define o nível mínimo de log.
     */
    static void setLevel(Level level);
    
    /**
     * @brief Ativa/desativa debug mode.
     */
    static void setDebugMode(bool enabled);
    
    /**
     * @brief Ativa/desativa cores.
     */
    static void setColors(bool enabled);
    
    /**
     * @brief Log trace (mais verboso).
     */
    static void trace(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief Log debug (informações de debug).
     */
    static void debug(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief Log info (informações gerais).
     */
    static void info(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief Log warning (avisos).
     */
    static void warning(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief Log error (erros).
     */
    static void error(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief Log fatal (erros críticos).
     */
    static void fatal(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief Flush o buffer do arquivo.
     */
    static void flush();
    
    /**
     * @brief Fecha o logger (flush + fecha arquivo).
     */
    static void shutdown();

private:
    static Level minLevel_;
    static bool useColors_;
    static bool debugMode_;
    static std::unique_ptr<std::ofstream> logFile_;
    static std::mutex mutex_;
    
    // Cores ANSI
    struct Colors {
        static constexpr const char* RESET = "\033[0m";
        static constexpr const char* BOLD = "\033[1m";
        
        static constexpr const char* BLACK = "\033[30m";
        static constexpr const char* RED = "\033[31m";
        static constexpr const char* GREEN = "\033[32m";
        static constexpr const char* YELLOW = "\033[33m";
        static constexpr const char* BLUE = "\033[34m";
        static constexpr const char* MAGENTA = "\033[35m";
        static constexpr const char* CYAN = "\033[36m";
        static constexpr const char* WHITE = "\033[37m";
        
        static constexpr const char* BRIGHT_BLACK = "\033[90m";
        static constexpr const char* BRIGHT_RED = "\033[91m";
        static constexpr const char* BRIGHT_GREEN = "\033[92m";
        static constexpr const char* BRIGHT_YELLOW = "\033[93m";
        static constexpr const char* BRIGHT_BLUE = "\033[94m";
        static constexpr const char* BRIGHT_MAGENTA = "\033[95m";
        static constexpr const char* BRIGHT_CYAN = "\033[96m";
        static constexpr const char* BRIGHT_WHITE = "\033[97m";
    };
    
    static void log(Level level, const std::string& message, const char* file, int line);
    static std::string levelToString(Level level);
    static const char* levelToColor(Level level);
    static std::string getTimestamp();
    static std::string extractFileName(const char* path);
};

} // namespace Core::Utils

// ==================== MACROS PARA DEBUG ====================

/**
 * Macros que automaticamente passam __FILE__ e __LINE__
 * Útil para debug mode
 */
#define LOG_TRACE(msg) Core::Utils::LoggerNew::trace(msg, __FILE__, __LINE__)
#define LOG_DEBUG(msg) Core::Utils::LoggerNew::debug(msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Core::Utils::LoggerNew::info(msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Core::Utils::LoggerNew::warning(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Core::Utils::LoggerNew::error(msg, __FILE__, __LINE__)
#define LOG_FATAL(msg) Core::Utils::LoggerNew::fatal(msg, __FILE__, __LINE__)

