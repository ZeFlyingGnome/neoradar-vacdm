#include <algorithm>
#include <string>

#include "core/DataManager.h"
#include "core/Server.h"
#include "log/Logger.h"
#include "utils/Number.h"
#include "utils/String.h"
#include "NeoVACDM.h"

using namespace vacdm;
using namespace vacdm::logging;
using namespace vacdm::core;
using namespace vacdm::utils;

namespace vacdm {
//bool vACDM::OnCompileCommand(const char *sCommandLine) {
//    std::string command(sCommandLine);

void NeoVACDM::RegisterCommand() {

    logger_->info("RegisterCommand");

    try
    {
        PluginSDK::Chat::CommandDefinition definition;
        definition.name = "vacdm";
        definition.description = "NeoVACDM command";
        definition.lastParameterHasSpaces = true;

        Chat::CommandParameter parameter;
        parameter.name = "vacdm";
        parameter.type = Chat::ParameterType::String; 
        parameter.required = true;
        definition.parameters.push_back(parameter);

        CommandProvider_ = std::make_shared<NeoVACDMCommandProvider>(this, logger_, chatAPI_, fsdAPI_);

        std::string commandId = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        if (!commandId.empty())
        {
            logger_->info("Successfully registered .vacdm command with ID: " + commandId);
            commandId_ = commandId;
        }
        else
        {
            logger_->error("Failed to register .vacdm command");
        }        
    }
    catch (const std::exception &ex)
    {
        logger_->error("Error registering command: " + std::string(ex.what()));
    }    
}

Chat::CommandResult NeoVACDMCommandProvider::Execute(
    const std::string &commandId,
    const std::vector<std::string> &args)
{
    size_t size = args.size();
    logger_->info("Executing command with " + std::to_string(size) + " arguments");

    if (args.empty())
    {
        std::string error = "Argument is required";
        neoVACDM_->DisplayMessage(error);
        return {false, error};
    }

    std::string command = args[0];
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);
    logger_->info("Processing command: " + command);

    if (std::string::npos != command.find("MASTER")) {
        std::string userIsNotEligibleMessage;
        bool userIsConnected = false;
        bool userIsInSweatbox = false;
        bool userIsObserver = false;
        auto connectionInfo = fsdAPI_->getConnection();
        if (connectionInfo) {
            userIsConnected = (*connectionInfo).isConnected;
            userIsInSweatbox = (*connectionInfo).serverType == Fsd::ServerType::Sweatbox;
            userIsObserver = (*connectionInfo).facility == Fsd::NetworkFacility::OBS;
        }
#ifndef DEV
        bool serverAllowsObsAsMaster = com::Server::instance().getServerConfig().allowMasterAsObserver;
#endif // !DEV
        bool serverAllowsSweatboxAsMaster = com::Server::instance().getServerConfig().allowMasterInSweatbox;

        if (!connectionInfo || !userIsConnected) {
            userIsNotEligibleMessage = "You are not logged in to the VATSIM network";
#ifndef DEV
        } else if (userIsObserver && !serverAllowsObsAsMaster) {
            userIsNotEligibleMessage = "You are logged in as Observer and Server does not allow Observers to be Master";
#endif // !DEV
        } else if (userIsInSweatbox && !serverAllowsSweatboxAsMaster) {
            userIsNotEligibleMessage =
                "You are logged in on a Sweatbox Server and Server does not allow Sweatbox connections";
        } else {
            // Clear all pilot data when switching to master mode
            DataManager::instance().clearAllPilotData();
            neoVACDM_->DisplayMessage("All pilot data cleared");

            neoVACDM_->DisplayMessage("Executing vACDM as the MASTER");
            logging::Logger::instance().log(logging::Logger::LogSender::vACDM, "Switched to MASTER", logging::Logger::LogLevel::Info);
            com::Server::instance().setMaster(true);
            
            return {true, std::nullopt};
        }

        neoVACDM_->DisplayMessage("Cannot upgrade to Master");
        neoVACDM_->DisplayMessage(userIsNotEligibleMessage);
        return {false, userIsNotEligibleMessage};
    } else if (std::string::npos != command.find("SLAVE")) {
        neoVACDM_->DisplayMessage("Executing vACDM as the SLAVE");
        logging::Logger::instance().log(logging::Logger::LogSender::vACDM, "Switched to SLAVE", logging::Logger::LogLevel::Info);
        com::Server::instance().setMaster(false);

        // Clear all pilot data when switching to slave mode
        DataManager::instance().clearAllPilotData();
        neoVACDM_->DisplayMessage("All pilot data cleared");
        return {true, std::nullopt};
    } else if (std::string::npos != command.find("RELOAD")) {
        neoVACDM_->reloadConfiguration();
        return {true, std::nullopt};
    } else if (std::string::npos != command.find("LOG")) {
        if (std::string::npos != command.find("LOGLEVEL")) {
            neoVACDM_->DisplayMessage(logging::Logger::instance().handleLogLevelCommand(command));
        } else {
            neoVACDM_->DisplayMessage(logging::Logger::instance().handleLogCommand(command));
        }
        return {true, std::nullopt};
    } else if (std::string::npos != command.find("UPDATERATE")) {
        const auto elements = vacdm::utils::String::splitString(command, " ");
        if (elements.size() != 2) {
            std::string error = "Usage: .vacdm UPDATERATE value";
            neoVACDM_->DisplayMessage(error);
            return {false, error};
        }
        if ((false == isNumber(elements[1])) ||
            (std::stoi(elements[1]) < minUpdateCycleSeconds || std::stoi(elements[1]) > maxUpdateCycleSeconds)) {
            std::string error = "Usage: .vacdm UPDATERATE value\nValue must be number between " + std::to_string(minUpdateCycleSeconds) + " and " +
                           std::to_string(maxUpdateCycleSeconds);
            neoVACDM_->DisplayMessage(error);
            return {false, error};
        }
        neoVACDM_->DisplayMessage(DataManager::instance().setUpdateCycleSeconds(std::stoi(elements[1])));
    }    
    else 
    {
        std::string error = "Unknown argument " + args[0];
        neoVACDM_->DisplayMessage(error);
        return {false, error};
    }

    return {true, std::nullopt};;
}
}  // namespace vacdm