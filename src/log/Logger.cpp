#include "Logger.h"

#ifdef DEBUG_BUILD
#include <Windows.h>

#include <iostream>
#endif

#include <algorithm>
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
    stream << std::format("{0:%Y%m%d%H%M%S}", std::chrono::utc_clock::now()) << ".vacdm";
#ifdef DEBUG_BUILD
    AllocConsole();
#pragma warning(push)
#pragma warning(disable : 6031)
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#pragma warning(pop)
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
#ifdef DEBUG_BUILD
                std::cout << logsetting->name << ": " << it->message << "\n";
#endif
            
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

std::pair<std::string,bool> Logger::handleLogCommand(std::string arg) {
    std::string usageString = "Usage: .vacdm log ON/OFF/DEBUG";

    std::transform(arg.begin(), arg.end(), arg.begin(), ::toupper);

    if ("ON" == arg) {
        this->enableLogging();
        return {"Enabled logging", true};
    } else if ("OFF" == arg) {
        this->disableLogging();
        return {"Disabled logging", true};
    } else if ("DEBUG" == arg) {
        std::lock_guard guard(this->m_logLock);
        if (false == this->m_LogAll) {
            this->m_LogAll = true;
            return {"Set all log levels to DEBUG", true};
        } else {
            this->m_LogAll = false;
            return {"Reset log levels, using previous settings", true};
        }
    }

    return {usageString, false};
}

std::pair<std::string,bool> Logger::handleLogLevelCommand(const std::vector<std::string> &args) {
    std::string sender = args[0];
    std::string newLevel = args[1];
    std::transform(sender.begin(), sender.end(), sender.begin(), ::toupper);

    std::lock_guard guard(this->m_logLock);
    auto logsetting = std::find_if(logSettings.begin(), logSettings.end(), [sender](const LogSetting &setting) {
        std::string uppercaseName = setting.name;
        std::transform(uppercaseName.begin(), uppercaseName.end(), uppercaseName.begin(), ::toupper);

        return uppercaseName == sender;
    });

    // sender not found
    if (logsetting == logSettings.end()) {
        return {"Sender " + sender + " not found. Available senders are " +
               std::accumulate(std::next(logSettings.begin()), logSettings.end(), logSettings.front().name,
                               [](std::string acc, const LogSetting &setting) { return acc + " " + setting.name; }), false};
    }

    // Modify logsetting by reference
    auto &logSettingRef = *logsetting;

    std::transform(newLevel.begin(), newLevel.end(), newLevel.begin(), ::toupper);

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
        return {"Invalid log level: " + newLevel, false};
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

    return {"Changed sender " + logSettingRef.name + " to " + newLevel, true};
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
