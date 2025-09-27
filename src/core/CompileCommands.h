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
        CommandProvider_ = std::make_shared<NeoVACDMCommandProvider>(this, logger_, chatAPI_, fsdAPI_);

        PluginSDK::Chat::CommandDefinition definition;
        definition.name = "vacdm help";
        definition.description = "Dispaly all available vACDM commands";
        definition.lastParameterHasSpaces = false;
		definition.parameters.clear();

        helpCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vacdm master";
        definition.description = "Set client as a VACDM master";
        definition.lastParameterHasSpaces = false;
		definition.parameters.clear();

        masterCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vacdm slave";
        definition.description = "Set client as a VACDM slave";
        definition.lastParameterHasSpaces = false;
		definition.parameters.clear();

        slaveCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vacdm reload";
        definition.description = "Reload VACDM configuration file";
        definition.lastParameterHasSpaces = false;
		definition.parameters.clear();

        reloadCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vacdm log";
        definition.description = "Activates or deactivates vACDM logging";
        definition.lastParameterHasSpaces = false;
		definition.parameters.clear();

        Chat::CommandParameter parameter;
        parameter.name = "LOG";
        parameter.type = Chat::ParameterType::String; 
        parameter.required = true;
        definition.parameters.push_back(parameter);

        logCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vacdm loglevel";
        definition.description = "Sets logging level for given sender for vACDM";
        definition.lastParameterHasSpaces = false;
		definition.parameters.clear();

        parameter.name = "SENDER";
        parameter.type = Chat::ParameterType::String; 
        parameter.required = true;
        definition.parameters.push_back(parameter);
        parameter.name = "LOGLEVEL";
        parameter.type = Chat::ParameterType::String; 
        parameter.required = true;
        definition.parameters.push_back(parameter);

        loglevelCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);

        definition.name = "vacdm updaterate";
        definition.description = "Update vACDM refresh rate";
        definition.lastParameterHasSpaces = false;
		definition.parameters.clear();

        parameter.name = "UPDATERATE";
        parameter.type = Chat::ParameterType::Number; 
        parameter.required = true;
        definition.parameters.push_back(parameter);

        updaterateCommandId_ = chatAPI_->registerCommand(definition.name, definition, CommandProvider_);        
  
    }
    catch (const std::exception &ex)
    {
        logger_->error("Error registering command: " + std::string(ex.what()));
    }    
}

inline void NeoVACDM::unRegisterCommand()
{
    if (CommandProvider_)
    {
        chatAPI_->unregisterCommand(helpCommandId_);
        chatAPI_->unregisterCommand(masterCommandId_);
        chatAPI_->unregisterCommand(slaveCommandId_);
        chatAPI_->unregisterCommand(reloadCommandId_);
        chatAPI_->unregisterCommand(logCommandId_);
        chatAPI_->unregisterCommand(loglevelCommandId_);
        chatAPI_->unregisterCommand(updaterateCommandId_);
        CommandProvider_.reset();
	}
}

Chat::CommandResult NeoVACDMCommandProvider::Execute(
    const std::string &commandId,
    const std::vector<std::string> &args)
{
    if (commandId == neoVACDM_->helpCommandId_)
    {
        neoVACDM_->DisplayMessage(".vacdm master");
        neoVACDM_->DisplayMessage(".vacdm slave");
        neoVACDM_->DisplayMessage(".vacdm reload");
        neoVACDM_->DisplayMessage(".vacdm log (ON/OFF/DEBUG)");
        neoVACDM_->DisplayMessage(".vacdm loglevel (vACDM/DataManager/Server/ConfigParser/Utils) (DEBUG/INFO/WARNING/ERROR/CRITICAL/SYSTEM/DISABLED)");
        neoVACDM_->DisplayMessage(".vacdm updaterate (1-10)");
    }
    else if (commandId == neoVACDM_->masterCommandId_) {
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
            neoVACDM_->GetDataManager()->clearAllPilotData();
            neoVACDM_->DisplayMessage("All pilot data cleared");

            neoVACDM_->DisplayMessage("Executing vACDM as the MASTER");
            logging::Logger::instance().log(logging::Logger::LogSender::vACDM, "Switched to MASTER", logging::Logger::LogLevel::Info);
            com::Server::instance().setMaster(true);
            
            return {true, std::nullopt};
        }

        neoVACDM_->DisplayMessage("Cannot upgrade to Master", false);
        neoVACDM_->DisplayMessage(userIsNotEligibleMessage, false);
        return {false, userIsNotEligibleMessage};
    } else if (commandId == neoVACDM_->slaveCommandId_) {
        neoVACDM_->DisplayMessage("Executing vACDM as the SLAVE");
        logging::Logger::instance().log(logging::Logger::LogSender::vACDM, "Switched to SLAVE", logging::Logger::LogLevel::Info);
        com::Server::instance().setMaster(false);

        // Clear all pilot data when switching to slave mode
        neoVACDM_->GetDataManager()->clearAllPilotData();
        neoVACDM_->DisplayMessage("All pilot data cleared");
        return {true, std::nullopt};
    } else if (commandId == neoVACDM_->reloadCommandId_) {
        neoVACDM_->reloadConfiguration();
        return {true, std::nullopt};
    } else if (commandId == neoVACDM_->loglevelCommandId_) {
        std::pair<std::string,bool> result = logging::Logger::instance().handleLogLevelCommand(args);
        neoVACDM_->DisplayMessage(result.first, result.second);
        return {true, std::nullopt};
    } else if (commandId == neoVACDM_->logCommandId_) {
        std::pair<std::string,bool> result = logging::Logger::instance().handleLogCommand(args[0]);
        neoVACDM_->DisplayMessage(result.first, result.second);
        return {true, std::nullopt};
    } else if (commandId == neoVACDM_->updaterateCommandId_) {
        std::string updaterate = args[0];
        if ((false == isNumber(updaterate)) ||
            (std::stoi(updaterate) < minUpdateCycleSeconds || std::stoi(updaterate) > maxUpdateCycleSeconds)) {
            std::string error = "Usage: .vacdm UPDATERATE value\nValue must be number between " + std::to_string(minUpdateCycleSeconds) + " and " +
                           std::to_string(maxUpdateCycleSeconds);
            neoVACDM_->DisplayMessage(error, false);
            return {false, error};
        }
        neoVACDM_->DisplayMessage(neoVACDM_->GetDataManager()->setUpdateCycleSeconds(std::stoi(updaterate)));
    }    
    else 
    {
        std::string error = "Invalid command. To list available commands use .vacdm help";
        neoVACDM_->DisplayMessage(error, false);
        return {false, error};
    }

    return {true, std::nullopt};;
}
}  // namespace vacdm