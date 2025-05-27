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
        definition.lastParameterHasSpaces = false;

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

Chat::ValidationResult NeoVACDMCommandProvider::ValidateParameters(
    const std::string &commandId,
    const std::vector<std::string> &args,
    bool strict)
{
    size_t size = args.size();
    logger_->info("Validating command with " + std::to_string(size) + " arguments");

    if (args.empty())
    {
        return {false, "Argument is required"};
    }

    std::string arg0 = args[0];
    std::transform(arg0.begin(), arg0.end(), arg0.begin(), ::tolower);

    if ( size==1 and (arg0 == "master" or arg0 == "slave" or arg0 == "reload") )
    {
        return {true, std::nullopt};
    }
    else if ( size==2 and (arg0 == "log" or arg0 == "loglevel" or arg0 == "updaterate") )
    {
        return {true, std::nullopt};
    }
    else return {false, "Unknown argument " + args[0]};
}

Chat::CommandResult NeoVACDMCommandProvider::Execute(
    const std::string &commandId,
    const std::vector<std::string> &args)
{
    size_t size = args.size();
    logger_->info("Executing command with " + std::to_string(size) + " arguments");

    if (args.empty())
    {
        return {false, "Argument is required"};
    }

    std::string arg0 = args[0];
    std::transform(arg0.begin(), arg0.end(), arg0.begin(), ::tolower);

    if (arg0 == "master")
    {
        std::string userIsNotEligibleMessage;
        auto connectionInfo = fsdAPI_->getConnection();
        if (connectionInfo) {
            bool userIsConnected = (*connectionInfo).isConnected;
            bool userIsInSweatbox = (*connectionInfo).serverType == Fsd::ServerType::Sweatbox;
            bool userIsObserver = (*connectionInfo).facility == Fsd::NetworkFacility::OBS;
            bool serverAllowsObsAsMaster = com::Server::instance().getServerConfig().allowMasterAsObserver;
            bool serverAllowsSweatboxAsMaster = com::Server::instance().getServerConfig().allowMasterInSweatbox;

            if (!userIsConnected) {
                userIsNotEligibleMessage = "You are not logged in to the VATSIM network";
            } else if (userIsObserver && !serverAllowsObsAsMaster) {
                userIsNotEligibleMessage = "You are logged in as Observer and Server does not allow Observers to be Master";
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
        }
        else {
            userIsNotEligibleMessage = "Not able to retrieve connection information";
        }

        neoVACDM_->DisplayMessage("Cannot upgrade to Master");
        neoVACDM_->DisplayMessage(userIsNotEligibleMessage);
        return {false, userIsNotEligibleMessage};
    }
    else if (arg0 == "slave")
    {
        neoVACDM_->DisplayMessage("Executing vACDM as the SLAVE");
        logging::Logger::instance().log(logging::Logger::LogSender::vACDM, "Switched to SLAVE", logging::Logger::LogLevel::Info);
        com::Server::instance().setMaster(false);

        // Clear all pilot data when switching to slave mode
        DataManager::instance().clearAllPilotData();
        neoVACDM_->DisplayMessage("All pilot data cleared");
    }
    else if (arg0 == "reload")
    {
        neoVACDM_->reloadConfiguration();
    }
    else if (arg0 == "loglevel" || arg0 == "log")
    {
        std::string command = "vacdm ";
        for (size_t i = 1; i < size; ++i)command += args[i] + " ";

        if (arg0 == "loglevel")
            neoVACDM_->DisplayMessage(logging::Logger::instance().handleLogLevelCommand(command));
        else if (arg0 == "log")
            neoVACDM_->DisplayMessage(logging::Logger::instance().handleLogCommand(command));
    }
    else if (arg0 == "updaterate")
    {
        if (size != 2) {
            std::string error = "Usage: .vacdm UPDATERATE value";
            neoVACDM_->DisplayMessage(error);
            return {false, error};
        }
        if (false == isNumber(args[1]) ||
            std::stoi(args[1]) < minUpdateCycleSeconds && std::stoi(args[1]) > maxUpdateCycleSeconds) {
            std::string error = "Usage: .vacdm UPDATERATE value\nValue must be number between " + std::to_string(minUpdateCycleSeconds) + " and " +
                           std::to_string(maxUpdateCycleSeconds);
            neoVACDM_->DisplayMessage(error);
            return {false, error};
        }

        neoVACDM_->DisplayMessage(DataManager::instance().setUpdateCycleSeconds(std::stoi(args[1])));
    }    
    else 
    {
        std::string error = "Unknown argument " + args[0];
        logger_->info(error);
        return {false, error};
    }

    return {true, std::nullopt};;
}



//#pragma warning(push)
//#pragma warning(disable : 4244)
//    std::transform(command.begin(), command.end(), command.begin(), ::toupper);
//#pragma warning(pop)

    // only handle commands containing ".vacdm"
    //if (0 != command.find(".VACDM")) return false;

}  // namespace vacdm