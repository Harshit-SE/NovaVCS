/**
 * @file logger.hpp
 * @brief Thread-safe logging facility
 */
#pragma once
#include <string>
#include <iostream>

namespace nova::core {

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

/**
 * @class Logger
 * @brief Singleton logger for standard output reporting.
 */
class Logger {
public:
    static Logger& instance();
    void setLevel(LogLevel level);
    void log(LogLevel level, const std::string& msg);
    
    static void info(const std::string& msg) { instance().log(LogLevel::INFO, msg); }
    static void error(const std::string& msg) { instance().log(LogLevel::ERROR, msg); }
    static void debug(const std::string& msg) { instance().log(LogLevel::DEBUG, msg); }
private:
    Logger() = default;
    LogLevel currentLevel_ = LogLevel::INFO;
};

} // namespace nova::core
