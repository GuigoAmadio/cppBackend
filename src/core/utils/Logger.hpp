#pragma once

#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Core::Utils {

/**
 * Logger simples
 * 
 * Em produção, usar biblioteca como spdlog
 */
class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERR  // Renomeado de ERROR para evitar conflito com macro do Windows
    };
    
    static void setLevel(Level level) {
        minLevel_ = level;
    }
    
    static void debug(const std::string& message) {
        log(Level::DEBUG, message);
    }
    
    static void info(const std::string& message) {
        log(Level::INFO, message);
    }
    
    static void warning(const std::string& message) {
        log(Level::WARNING, message);
    }
    
    static void error(const std::string& message) {
        log(Level::ERR, message);
    }
    
private:
    static Level minLevel_;
    
    static void log(Level level, const std::string& message) {
        if (level < minLevel_) return;
        
        // Timestamp
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream oss;
        oss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
        
        // Level
        switch (level) {
            case Level::DEBUG:
                oss << "[DEBUG] ";
                break;
            case Level::INFO:
                oss << "[INFO]  ";
                break;
            case Level::WARNING:
                oss << "[WARN]  ";
                break;
            case Level::ERR:
                oss << "[ERROR] ";
                break;
        }
        
        // Message
        oss << message;
        
        // Output
        if (level == Level::ERR) {
            std::cerr << oss.str() << std::endl;
        } else {
            std::cout << oss.str() << std::endl;
        }
    }
};

// Definição static
inline Logger::Level Logger::minLevel_ = Logger::Level::INFO;

} // namespace Core::Utils

