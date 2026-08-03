#include "nova/core/logger.hpp"

namespace nova::core {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(LogLevel level) {
    currentLevel_ = level;
}

void Logger::log(LogLevel level, const std::string& msg) {
    if (level >= currentLevel_) {
        std::string prefix;
        switch (level) {
            case LogLevel::DEBUG: prefix = "[DEBUG] "; break;
            case LogLevel::INFO:  prefix = "[INFO] "; break;
            case LogLevel::WARN:  prefix = "[WARN] "; break;
            case LogLevel::ERROR: prefix = "[ERROR] "; break;
        }
        
        if (level == LogLevel::ERROR) {
            std::cerr << prefix << msg << std::endl;
        } else {
            std::cout << prefix << msg << std::endl;
        }
    }
}

}
