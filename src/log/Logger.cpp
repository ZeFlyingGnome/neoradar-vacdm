#include "Logger.h"

#include <algorithm>
#include <chrono>
#include <format>
#include <numeric>

#include "spdlog/sinks/basic_file_sink.h"
#include "utils/String.h"

using namespace vacdm::logging;

Logger::Logger() {
    // Create log file with timestamp
    std::string logFilename = std::format("{0:%Y%m%d%H%M%S}.vacdm.log", std::chrono::utc_clock::now());

    // Create a file sink for all builds
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilename, true);
    file_sink->set_level(spdlog::level::warn);  // Default log-level

    // Initialize loggers for different senders
    for (const auto &setting : logSettings) {
        // Create logger with both sinks
        auto logger = std::make_shared<spdlog::logger>(setting.name);

        // Add file sink for all loggers
        logger->sinks().push_back(file_sink);

        // Set the level based on the settings
        spdlog::level::level_enum level;
        switch (setting.minimumLevel) {
            case Debug:
                level = spdlog::level::debug;
                break;
            case Info:
                level = spdlog::level::info;
                break;
            case Warning:
                level = spdlog::level::warn;
                break;
            case Error:
                level = spdlog::level::err;
                break;
            case Critical:
                level = spdlog::level::critical;
                break;
            case System:
                level = spdlog::level::info;
                break;  // Map system to info
            case Disabled:
                level = spdlog::level::off;
                break;
            default:
                level = spdlog::level::info;
        }

        logger->set_level(level);
        logger->flush_on(spdlog::level::warn);  // Flush on warning and above
        spdlog::register_logger(logger);
        loggers.push_back(logger);
    }

    enableLogging();
}

Logger::~Logger() {
    // spdlog will clean up loggers and sinks automatically
    spdlog::shutdown();
}

std::shared_ptr<spdlog::logger> Logger::getLogger(const LogSender &sender) {
    auto logsetting = std::find_if(logSettings.begin(), logSettings.end(),
                                   [sender](const LogSetting &setting) { return setting.sender == sender; });

    if (logsetting != logSettings.end()) {
        return spdlog::get(logsetting->name);
    }

    // Return default logger if not found
    return spdlog::default_logger();
}

void Logger::log(const LogSender &sender, const std::string &message, const LogLevel loglevel) {
    std::lock_guard guard(this->m_logLock);
    if (!loggingEnabled) return;

    auto logger = getLogger(sender);
    if (!logger) return;

    switch (loglevel) {
        case Debug:
            logger->debug(message);
            break;
        case Info:
            logger->info(message);
            break;
        case Warning:
            logger->warn(message);
            break;
        case Error:
            logger->error(message);
            break;
        case Critical:
            logger->critical(message);
            break;
        case System:
            logger->info("[SYSTEM] {}", message);
            break;
        default:
            break;
    }
}

std::string Logger::handleLogCommand(std::string command) {
    auto elements = vacdm::utils::String::splitString(command, " ");

    std::string usageString = "Usage: .vacdm LOG ON/OFF/DEBUG";
    if (elements.size() != 3) return usageString;

    if ("ON" == elements[2]) {
        this->enableLogging();
        return "Enabled logging";
    } else if ("OFF" == elements[2]) {
        this->disableLogging();
        return "Disabled logging";
    } else if ("DEBUG" == elements[2]) {
        std::lock_guard guard(this->m_logLock);
        if (false == this->m_LogAll) {
            this->m_LogAll = true;
            // Set all loggers to debug level
            for (const auto &logger : loggers) {
                logger->set_level(spdlog::level::debug);
            }
            return "Set all log levels to DEBUG";
        } else {
            this->m_LogAll = false;
            // Reset loggers to their previous settings
            for (size_t i = 0; i < loggers.size() && i < logSettings.size(); ++i) {
                spdlog::level::level_enum level;
                switch (logSettings[i].minimumLevel) {
                    case Debug:
                        level = spdlog::level::debug;
                        break;
                    case Info:
                        level = spdlog::level::info;
                        break;
                    case Warning:
                        level = spdlog::level::warn;
                        break;
                    case Error:
                        level = spdlog::level::err;
                        break;
                    case Critical:
                        level = spdlog::level::critical;
                        break;
                    case System:
                        level = spdlog::level::info;
                        break;
                    case Disabled:
                        level = spdlog::level::off;
                        break;
                    default:
                        level = spdlog::level::info;
                }
                loggers[i]->set_level(level);
            }
            return "Reset log levels, using previous settings";
        }
    }

    return usageString;
}

std::string Logger::handleLogLevelCommand(std::string command) {
    const auto elements = vacdm::utils::String::splitString(command, " ");
    if (elements.size() != 4) {
        return "Usage: .vacdm LOGLEVEL sender loglevel";
    }

    std::string sender = elements[2];
    std::string newLevel = elements[3];

    std::lock_guard guard(this->m_logLock);
    auto logsetting = std::find_if(logSettings.begin(), logSettings.end(), [sender](const LogSetting &setting) {
        std::string uppercaseName = setting.name;
        std::transform(uppercaseName.begin(), uppercaseName.end(), uppercaseName.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        return uppercaseName == sender;
    });

    // sender not found
    if (logsetting == logSettings.end()) {
        return "Sender " + sender + " not found. Available senders are " +
               std::accumulate(std::next(logSettings.begin()), logSettings.end(), logSettings.front().name,
                               [](std::string acc, const LogSetting &setting) { return acc + " " + setting.name; });
    }

    // Modify logsetting by reference
    auto &logSettingRef = *logsetting;
    auto logger = spdlog::get(logSettingRef.name);
    if (!logger) return "Logger not found";

    std::transform(newLevel.begin(), newLevel.end(), newLevel.begin(), [](unsigned char c) { return std::toupper(c); });

    spdlog::level::level_enum spdLogLevel;
    if (newLevel == "DEBUG") {
        logSettingRef.minimumLevel = LogLevel::Debug;
        spdLogLevel = spdlog::level::debug;
    } else if (newLevel == "INFO") {
        logSettingRef.minimumLevel = LogLevel::Info;
        spdLogLevel = spdlog::level::info;
    } else if (newLevel == "WARNING") {
        logSettingRef.minimumLevel = LogLevel::Warning;
        spdLogLevel = spdlog::level::warn;
    } else if (newLevel == "ERROR") {
        logSettingRef.minimumLevel = LogLevel::Error;
        spdLogLevel = spdlog::level::err;
    } else if (newLevel == "CRITICAL") {
        logSettingRef.minimumLevel = LogLevel::Critical;
        spdLogLevel = spdlog::level::critical;
    } else if (newLevel == "SYSTEM") {
        logSettingRef.minimumLevel = LogLevel::System;
        spdLogLevel = spdlog::level::info;
    } else if (newLevel == "DISABLED") {
        logSettingRef.minimumLevel = LogLevel::Disabled;
        spdLogLevel = spdlog::level::off;
    } else {
        return "Invalid log level: " + newLevel;
    }

    logger->set_level(spdLogLevel);

    // check if at least one sender is set to log
    bool enableLogging = false;
    for (auto logSetting : logSettings) {
        if (logSetting.minimumLevel != LogLevel::Disabled) {
            enableLogging = true;
            break;
        }
    }
    this->loggingEnabled = enableLogging;

    return "Changed sender " + sender + " to " + newLevel;
}

void Logger::enableLogging() {
    std::lock_guard guard(this->m_logLock);
    this->loggingEnabled = true;
}

void Logger::disableLogging() {
    std::lock_guard guard(this->m_logLock);
    this->loggingEnabled = false;
}

Logger &Logger::instance() {
    static Logger __instance;
    return __instance;
}
