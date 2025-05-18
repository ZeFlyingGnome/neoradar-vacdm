#pragma once

#include <array>
#include <chrono>
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

namespace vacdm::logging {

/**
 * @brief Logger namespace replacement for the original Logger class
 * This provides direct access to spdlog functionality through a clean interface
 */
namespace SpdLogger {

// Define logger names that match the original LogSender enum
inline constexpr std::array<const char*, 5> LoggerNames = {"vACDM", "DataManager", "Server", "ConfigParser", "Utils"};

enum class LogSender { vACDM = 0, DataManager = 1, Server = 2, ConfigParser = 3, Utils = 4 };

enum class LogLevel { Debug, Info, Warning, Error, Critical, System, Disabled };

struct LoggerSetting {
    LogSender sender;
    std::string name;
    LogLevel minLevel;
};

// Static variables
extern bool loggingEnabled;
extern bool debugMode;
extern std::vector<LoggerSetting> loggerSettings;
extern std::mutex logMutex;

/**
 * @brief Initialize all loggers
 * This function should be called once at the start of the application
 */
void initialize();

/**
 * @brief Shutdown all loggers
 */
void shutdown();

/**
 * @brief Convert LogLevel to spdlog level enum
 */
spdlog::level::level_enum toLevelEnum(LogLevel level);

/**
 * @brief Get logger by name
 * @param sender The logger sender identifier
 * @return The logger instance
 */
inline std::shared_ptr<spdlog::logger> getLogger(LogSender sender) {
    const char* name = LoggerNames[static_cast<int>(sender)];
    auto logger = spdlog::get(name);
    if (!logger) {
        // Return the default logger as fallback
        return spdlog::default_logger();
    }
    return logger;
}

/**
 * @brief Log a message
 * @param sender The logger sender
 * @param message The message to log
 * @param level The log level
 */
inline void log(LogSender sender, const std::string& message, LogLevel level) {
    std::lock_guard<std::mutex> guard(logMutex);

    if (!loggingEnabled) return;

    auto logger = getLogger(sender);

    switch (level) {
        case LogLevel::Debug:
            logger->debug(message);
            break;
        case LogLevel::Info:
            logger->info(message);
            break;
        case LogLevel::Warning:
            logger->warn(message);
            break;
        case LogLevel::Error:
            logger->error(message);
            break;
        case LogLevel::Critical:
            logger->critical(message);
            break;
        case LogLevel::System:
            logger->info("[SYSTEM] {}", message);
            break;
        default:
            break;
    }
}

/**
 * @brief Set log level for a specific logger
 * @param sender The logger sender
 * @param level The log level to set
 */
inline void setLogLevel(LogSender sender, LogLevel level) {
    std::lock_guard<std::mutex> guard(logMutex);
    auto logger = getLogger(sender);
    logger->set_level(toLevelEnum(level));

    // Update settings array
    for (auto& setting : loggerSettings) {
        if (setting.sender == sender) {
            setting.minLevel = level;
            break;
        }
    }
}

/**
 * @brief Set all loggers to debug level
 */
inline void setAllDebug() {
    std::lock_guard<std::mutex> guard(logMutex);
    for (const auto& name : LoggerNames) {
        auto logger = spdlog::get(name);
        if (logger) {
            logger->set_level(spdlog::level::debug);
        }
    }
    debugMode = true;
}

/**
 * @brief Enable logging for all loggers
 */
inline void enableLogging() {
    std::lock_guard<std::mutex> guard(logMutex);
    loggingEnabled = true;
}

/**
 * @brief Disable logging for all loggers
 */
inline void disableLogging() {
    std::lock_guard<std::mutex> guard(logMutex);
    loggingEnabled = false;
}

/**
 * @brief Parse and handle log command
 * @param command The command string
 * @return Result message
 */
std::string handleLogCommand(const std::string& command); /**
                                                           * @brief Parse and handle log level command
                                                           * @param command The command string
                                                           * @return Result message
                                                           */
std::string handleLogLevelCommand(const std::string& command);

}  // namespace SpdLogger

// For compatibility with existing code, create an alias for the old Logger class
namespace Logger {
using LogSender = SpdLogger::LogSender;
using LogLevel = SpdLogger::LogLevel;

// Static wrapper for backward compatibility
inline void log(LogSender sender, const std::string& message, LogLevel level) {
    SpdLogger::log(sender, message, level);
}

// Compatibility methods
inline std::string handleLogCommand(const std::string& command) { return SpdLogger::handleLogCommand(command); }

inline std::string handleLogLevelCommand(const std::string& command) {
    return SpdLogger::handleLogLevelCommand(command);
}

// Singleton-like accessor for backward compatibility
// Returns a dummy object that redirects to static methods
struct LoggerWrapper {
    void log(LogSender sender, const std::string& message, LogLevel level) { SpdLogger::log(sender, message, level); }

    std::string handleLogCommand(const std::string& command) { return SpdLogger::handleLogCommand(command); }

    std::string handleLogLevelCommand(const std::string& command) { return SpdLogger::handleLogLevelCommand(command); }
};

inline LoggerWrapper& instance() {
    static LoggerWrapper wrapper;
    return wrapper;
}
}  // namespace Logger

}  // namespace vacdm::logging
