#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

namespace vacdm::logging {

class Logger {
   public:
    enum LogSender {
        vACDM,
        DataManager,
        Server,
        ConfigParser,
        Utils,
    };

    enum LogLevel {
        Debug,
        Info,
        Warning,
        Error,
        Critical,
        System,
        Disabled,
    };

    struct LogSetting {
        LogSender sender;
        std::string name;
        LogLevel minimumLevel;
    };

   private:
    Logger();
    std::shared_ptr<spdlog::logger> getLogger(const LogSender &sender);
    std::vector<LogSetting> logSettings = {
        {vACDM, "vACDM", Disabled},   {DataManager, "DataManager", Disabled},
        {Server, "Server", Disabled}, {ConfigParser, "ConfigParser", Disabled},
        {Utils, "Utils", Disabled},
    };
    bool m_LogAll = false;

    std::mutex m_logLock;
    bool loggingEnabled = false;
    std::vector<std::shared_ptr<spdlog::logger>> loggers;

    void enableLogging();
    void disableLogging();

   public:
    ~Logger();
    void log(const LogSender &sender, const std::string &message, const LogLevel loglevel);
    std::string handleLogCommand(std::string command);
    std::string handleLogLevelCommand(std::string command);
    static Logger &instance();
};

}  // namespace vacdm::logging