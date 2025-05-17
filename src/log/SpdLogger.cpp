#include "SpdLogger.h"

#include <algorithm>
#include <cctype>
#include <numeric>
#include <vector>

#include "spdlog/sinks/basic_file_sink.h"
#include "utils/String.h"

namespace vacdm::logging {

// Initialize static members
bool SpdLogger::loggingEnabled = true;
bool SpdLogger::debugMode = false;
std::vector<SpdLogger::LoggerSetting> SpdLogger::loggerSettings;
std::mutex SpdLogger::logMutex;

void SpdLogger::initialize() {
    std::string logFilename = std::format("{0:%Y%m%d%H%M%S}.vacdm.log", std::chrono::utc_clock::now());

    // Create a common file sink for all loggers
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilename, true);
    file_sink->set_level(spdlog::level::warn);  // Default level

    // Initialize logger settings
    loggerSettings.clear();
    for (size_t i = 0; i < LoggerNames.size(); ++i) {
        loggerSettings.push_back({static_cast<LogSender>(i), LoggerNames[i], LogLevel::Info});
    }

    // Initialize loggers for different senders
    for (const auto& setting : loggerSettings) {
        auto logger = std::make_shared<spdlog::logger>(setting.name);
        logger->sinks().push_back(file_sink);

        // Set level based on setting
        spdlog::level::level_enum level = toLevelEnum(setting.minLevel);
        spdlog::flush_every(std::chrono::seconds(1));
        logger->set_level(level);
        logger->flush_on(spdlog::level::warn);
        spdlog::register_logger(logger);
    }

    // Set the default pattern
    spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e [%n] [%l] %v");
}

void SpdLogger::shutdown() { spdlog::shutdown(); }

spdlog::level::level_enum SpdLogger::toLevelEnum(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return spdlog::level::debug;
        case LogLevel::Info:
            return spdlog::level::info;
        case LogLevel::Warning:
            return spdlog::level::warn;
        case LogLevel::Error:
            return spdlog::level::err;
        case LogLevel::Critical:
            return spdlog::level::critical;
        case LogLevel::System:
            return spdlog::level::info;
        case LogLevel::Disabled:
            return spdlog::level::off;
        default:
            return spdlog::level::info;
    }
}

std::string SpdLogger::handleLogCommand(const std::string& command) {
    auto elements = vacdm::utils::String::splitString(command, " ");

    std::string usageString = "Usage: .vacdm LOG ON/OFF/DEBUG";
    if (elements.size() != 3) return usageString;

    std::lock_guard<std::mutex> guard(logMutex);

    if ("ON" == elements[2]) {
        loggingEnabled = true;
        return "Enabled logging";
    } else if ("OFF" == elements[2]) {
        loggingEnabled = false;
        return "Disabled logging";
    } else if ("DEBUG" == elements[2]) {
        if (!debugMode) {
            debugMode = true;
            // Set all loggers to debug level
            for (const auto& name : LoggerNames) {
                auto logger = spdlog::get(name);
                if (logger) {
                    logger->set_level(spdlog::level::debug);
                }
            }
            return "Set all log levels to DEBUG";
        } else {
            debugMode = false;
            // Reset loggers to their previous settings
            for (const auto& setting : loggerSettings) {
                auto logger = spdlog::get(setting.name);
                if (logger) {
                    logger->set_level(toLevelEnum(setting.minLevel));
                }
            }
            return "Reset log levels, using previous settings";
        }
    }

    return usageString;
}

// For more complex parsing of log commands
std::string SpdLogger::handleLogLevelCommand(const std::string& command) {
    // Split the command into parts
    const auto elements = vacdm::utils::String::splitString(command, " ");
    if (elements.size() != 4) {
        return "Usage: .vacdm LOGLEVEL sender loglevel";
    }

    std::string senderStr = elements[2];
    std::string levelStr = elements[3];

    // Convert sender string to uppercase
    std::transform(senderStr.begin(), senderStr.end(), senderStr.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    // Convert level string to uppercase
    std::transform(levelStr.begin(), levelStr.end(), levelStr.begin(), [](unsigned char c) { return std::toupper(c); });

    // Find the logger by name
    LogSender sender = LogSender::vACDM;  // Default
    bool senderFound = false;

    for (size_t i = 0; i < LoggerNames.size(); ++i) {
        std::string upperName = LoggerNames[i];
        std::transform(upperName.begin(), upperName.end(), upperName.begin(),
                       [](unsigned char c) { return std::toupper(c); });

        if (upperName == senderStr) {
            sender = static_cast<LogSender>(i);
            senderFound = true;
            break;
        }
    }

    // If sender not found
    if (!senderFound) {
        std::string availableSenders;
        for (size_t i = 0; i < LoggerNames.size(); ++i) {
            if (i > 0) availableSenders += " ";
            availableSenders += LoggerNames[i];
        }
        return "Sender " + senderStr + " not found. Available senders are " + availableSenders;
    }

    // Parse log level
    LogLevel level;
    if (levelStr == "DEBUG") {
        level = LogLevel::Debug;
    } else if (levelStr == "INFO") {
        level = LogLevel::Info;
    } else if (levelStr == "WARNING") {
        level = LogLevel::Warning;
    } else if (levelStr == "ERROR") {
        level = LogLevel::Error;
    } else if (levelStr == "CRITICAL") {
        level = LogLevel::Critical;
    } else if (levelStr == "SYSTEM") {
        level = LogLevel::System;
    } else if (levelStr == "DISABLED") {
        level = LogLevel::Disabled;
    } else {
        return "Invalid log level: " + levelStr;
    }

    // Set the log level
    std::lock_guard<std::mutex> guard(logMutex);

    // Update the logger setting
    for (auto& setting : loggerSettings) {
        if (setting.sender == sender) {
            setting.minLevel = level;
            break;
        }
    }

    // Update the actual logger level
    auto logger = spdlog::get(LoggerNames[static_cast<int>(sender)]);
    if (logger) {
        logger->set_level(toLevelEnum(level));
    }

    return "Changed sender " + senderStr + " to " + levelStr;
}

}  // namespace vacdm::logging
