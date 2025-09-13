#include "Logger.h"

#include <chrono>
#include <numeric>

#include "utils/String.h"

using namespace std::chrono_literals;
using namespace vacdm::logging;

static const char __loggingTable[] =
    "CREATE TABLE messages( \
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, \
    sender TEXT, \
    level INT, \
    message TEXT \
);";
static const std::string __insertMessage = "INSERT INTO messages VALUES (CURRENT_TIMESTAMP, @1, @2, @3)";

Logger::Logger() {
#ifdef DEV
    this->enableLogging();
#endif
    this->m_logWriter = std::thread(&Logger::run, this);
}

Logger::~Logger() {
    this->m_stop = true;
    this->m_logWriter.join();
}

void Logger::run() {
    while (true) {
        if (m_stop) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // obtain a copy of the logs, clear the log list to minimize lock time
        this->m_logLock.lock();
        auto logs = m_asynchronousLogs;
        m_asynchronousLogs.clear();
        this->m_logLock.unlock();

        auto it = logs.begin();
        while (it != logs.end()) {
            auto logsetting = std::find_if(logSettings.begin(), logSettings.end(),
                                           [it](const LogSetting &setting) { return setting.sender == it->sender; });

            if (logsetting != logSettings.end() && it->loglevel >= logsetting->minimumLevel &&
                false == this->m_LogAll) {
            
                if (vacdmLogger_)
                {
                    switch (it->loglevel)
                    {
                        case Info:
                            vacdmLogger_->info(it->message.c_str());
                            break;
                        case Debug:
                            vacdmLogger_->debug(it->message.c_str());
                            break;
                        case Warning:
                            vacdmLogger_->warning(it->message.c_str());
                            break;
                        case Error:
                            vacdmLogger_->error(it->message.c_str());
                            break;
                        case Critical:
                            vacdmLogger_->fatal(it->message.c_str());
                            break;
                        case System:
                            vacdmLogger_->verbose(it->message.c_str());
                            break;
                    }
                } 
            }
            it = logs.erase(it);
        }
    }
}

void Logger::log(const LogSender &sender, const std::string &message, const LogLevel loglevel) {
    std::lock_guard guard(this->m_logLock);

    if (true == this->loggingEnabled) 
    {
        m_asynchronousLogs.push_back({sender, message, loglevel});
    }
}

std::string Logger::handleLogCommand(std::string command) {
    auto elements = vacdm::utils::String::splitString(command, " ");

    std::string usageString = "Usage: .vacdm LOG ON/OFF/DEBUG";
    if (elements.size() != 2) return usageString;

    if ("ON" == elements[1]) {
        this->enableLogging();
        return "Enabled logging";
    } else if ("OFF" == elements[1]) {
        this->disableLogging();
        return "Disabled logging";
    } else if ("DEBUG" == elements[1]) {
        std::lock_guard guard(this->m_logLock);
        if (false == this->m_LogAll) {
            this->m_LogAll = true;
            return "Set all log levels to DEBUG";
        } else {
            this->m_LogAll = false;
            return "Reset log levels, using previous settings";
        }
    }

    return usageString;
}

std::string Logger::handleLogLevelCommand(std::string command) {
    const auto elements = vacdm::utils::String::splitString(command, " ");
    if (elements.size() != 3) {
        return "Usage: .vacdm LOGLEVEL sender loglevel";
    }

    std::string sender = elements[1];
    std::string newLevel = elements[2];

    std::lock_guard guard(this->m_logLock);
    auto logsetting = std::find_if(logSettings.begin(), logSettings.end(), [sender](const LogSetting &setting) {
        std::string uppercaseName = setting.name;
#pragma warning(push)
#pragma warning(disable : 4244)
        std::transform(uppercaseName.begin(), uppercaseName.end(), uppercaseName.begin(), ::toupper);
#pragma warning(pop)
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

#pragma warning(push)
#pragma warning(disable : 4244)
    std::transform(newLevel.begin(), newLevel.end(), newLevel.begin(), ::toupper);
#pragma warning(pop)

    if (newLevel == "DEBUG") {
        logSettingRef.minimumLevel = LogLevel::Debug;
    } else if (newLevel == "INFO") {
        logSettingRef.minimumLevel = LogLevel::Info;
    } else if (newLevel == "WARNING") {
        logSettingRef.minimumLevel = LogLevel::Warning;
    } else if (newLevel == "ERROR") {
        logSettingRef.minimumLevel = LogLevel::Error;
    } else if (newLevel == "CRITICAL") {
        logSettingRef.minimumLevel = LogLevel::Critical;
    } else if (newLevel == "SYSTEM") {
        logSettingRef.minimumLevel = LogLevel::System;
    } else if (newLevel == "DISABLED") {
        logSettingRef.minimumLevel = LogLevel::Disabled;
    } else {
        return "Invalid log level: " + newLevel;
    }

    // check if at least one sender is set to log
    bool enableLogging = false;
    for (auto logSetting : logSettings) {
        if (logsetting->minimumLevel != LogLevel::Disabled) {
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
