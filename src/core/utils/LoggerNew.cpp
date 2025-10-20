#include "LoggerNew.hpp"
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Core::Utils {

// ==================== STATIC MEMBERS ====================

LoggerNew::Level LoggerNew::minLevel_ = LoggerNew::Level::INFO;
bool LoggerNew::useColors_ = true;
bool LoggerNew::debugMode_ = false;
std::unique_ptr<std::ofstream> LoggerNew::logFile_ = nullptr;
std::mutex LoggerNew::mutex_;

// ==================== PUBLIC METHODS ====================

void LoggerNew::init(
    const std::string& logFile,
    Level minLevel,
    bool useColors,
    bool debugMode
) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    minLevel_ = minLevel;
    useColors_ = useColors;
    debugMode_ = debugMode;
    
    // Habilitar cores ANSI no Windows 10+
#ifdef _WIN32
    if (useColors_) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (GetConsoleMode(hConsole, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hConsole, mode);
        }
    }
#endif
    
    // Abrir arquivo de log se especificado
    if (!logFile.empty()) {
        logFile_ = std::make_unique<std::ofstream>(logFile, std::ios::app);
        if (!logFile_->is_open()) {
            std::cerr << "ERRO: Falha ao abrir arquivo de log: " << logFile << std::endl;
            logFile_ = nullptr;
        } else {
            // Log inicial
            if (logFile_) {
                *logFile_ << "\n========================================\n";
                *logFile_ << "Logger iniciado em " << getTimestamp() << "\n";
                *logFile_ << "========================================\n";
                logFile_->flush();
            }
        }
    }
}

void LoggerNew::setLevel(Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    minLevel_ = level;
}

void LoggerNew::setDebugMode(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    debugMode_ = enabled;
}

void LoggerNew::setColors(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    useColors_ = enabled;
}

void LoggerNew::trace(const std::string& message, const char* file, int line) {
    log(Level::TRACE, message, file, line);
}

void LoggerNew::debug(const std::string& message, const char* file, int line) {
    log(Level::DEBUG, message, file, line);
}

void LoggerNew::info(const std::string& message, const char* file, int line) {
    log(Level::INFO, message, file, line);
}

void LoggerNew::warning(const std::string& message, const char* file, int line) {
    log(Level::WARNING, message, file, line);
}

void LoggerNew::error(const std::string& message, const char* file, int line) {
    log(Level::ERR, message, file, line);
}

void LoggerNew::fatal(const std::string& message, const char* file, int line) {
    log(Level::FATAL, message, file, line);
}

void LoggerNew::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_) {
        logFile_->flush();
    }
}

void LoggerNew::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_) {
        *logFile_ << "\n========================================\n";
        *logFile_ << "Logger encerrado em " << getTimestamp() << "\n";
        *logFile_ << "========================================\n";
        logFile_->flush();
        logFile_->close();
        logFile_ = nullptr;
    }
}

// ==================== PRIVATE METHODS ====================

void LoggerNew::log(Level level, const std::string& message, const char* file, int line) {
    if (level < minLevel_) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ostringstream oss;
    
    // Timestamp
    oss << "[" << getTimestamp() << "] ";
    
    // Level com cor
    const char* color = useColors_ ? levelToColor(level) : "";
    const char* reset = useColors_ ? Colors::RESET : "";
    
    oss << color << "[" << levelToString(level) << "]" << reset << " ";
    
    // Debug info (arquivo:linha) se habilitado
    if (debugMode_ && file && line > 0) {
        oss << Colors::BRIGHT_BLACK << "[" << extractFileName(file) << ":" << line << "]" << reset << " ";
    }
    
    // Mensagem
    oss << message;
    
    // Output para console
    if (level >= Level::ERR) {
        std::cerr << oss.str() << std::endl;
    } else {
        std::cout << oss.str() << std::endl;
    }
    
    // Output para arquivo (sem cores)
    if (logFile_) {
        std::ostringstream fileOss;
        fileOss << "[" << getTimestamp() << "] ";
        fileOss << "[" << levelToString(level) << "] ";
        if (debugMode_ && file && line > 0) {
            fileOss << "[" << extractFileName(file) << ":" << line << "] ";
        }
        fileOss << message;
        
        *logFile_ << fileOss.str() << std::endl;
    }
}

std::string LoggerNew::levelToString(Level level) {
    switch (level) {
        case Level::TRACE:   return "TRACE";
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO ";
        case Level::WARNING: return "WARN ";
        case Level::ERR:     return "ERROR";
        case Level::FATAL:   return "FATAL";
        default:             return "?????";
    }
}

const char* LoggerNew::levelToColor(Level level) {
    switch (level) {
        case Level::TRACE:   return Colors::BRIGHT_BLACK;   // Cinza
        case Level::DEBUG:   return Colors::CYAN;           // Ciano
        case Level::INFO:    return Colors::GREEN;          // Verde
        case Level::WARNING: return Colors::YELLOW;         // Amarelo
        case Level::ERR:     return Colors::RED;            // Vermelho
        case Level::FATAL:   return Colors::BRIGHT_RED;     // Vermelho brilhante
        default:             return Colors::WHITE;
    }
}

std::string LoggerNew::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count() % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms;
    
    return oss.str();
}

std::string LoggerNew::extractFileName(const char* path) {
    if (!path) return "";
    
    std::string fullPath(path);
    
    // Encontrar último separador (/ ou \)
    size_t pos = fullPath.find_last_of("/\\");
    if (pos != std::string::npos) {
        return fullPath.substr(pos + 1);
    }
    
    return fullPath;
}

} // namespace Core::Utils

