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

        CommandProvider_ = std::make_shared<NeoVACDMCommandProvider>(logger_, chatAPI_);

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

    if ( arg0 == "master" and size == 1)
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

    if ( arg0 == "master" and size == 1)
    {
        /*bool userIsConnected = this->GetConnectionType() != EuroScopePlugIn::CONNECTION_TYPE_NO;
        bool userIsInSweatbox = this->GetConnectionType() == EuroScopePlugIn::CONNECTION_TYPE_SWEATBOX;
        bool userIsObserver = std::string_view(this->ControllerMyself().GetCallsign()).ends_with("_OBS") == true ||
                              this->ControllerMyself().GetFacility() == 0;
        bool serverAllowsObsAsMaster = com::Server::instance().getServerConfig().allowMasterAsObserver;
        bool serverAllowsSweatboxAsMaster = com::Server::instance().getServerConfig().allowMasterInSweatbox;

        std::string userIsNotEligibleMessage;

        if (!userIsConnected) {
            userIsNotEligibleMessage = "You are not logged in to the VATSIM network";
        } else if (userIsObserver && !serverAllowsObsAsMaster) {
            userIsNotEligibleMessage = "You are logged in as Observer and Server does not allow Observers to be Master";
        } else if (userIsInSweatbox && !serverAllowsSweatboxAsMaster) {
            userIsNotEligibleMessage =
                "You are logged in on a Sweatbox Server and Server does not allow Sweatbox connections";
        } else {*/
            // Clear all pilot data when switching to master mode
            DataManager::instance().clearAllPilotData();
            DisplayMessage("All pilot data cleared");

            DisplayMessage("Executing vACDM as the MASTER");
            logging::Logger::instance().log(logging::Logger::LogSender::vACDM, "Switched to MASTER", logging::Logger::LogLevel::Info);
            com::Server::instance().setMaster(true);

        return {true, std::nullopt};
    }
    else return {false, "Unknown argument " + args[0]};
}



//#pragma warning(push)
//#pragma warning(disable : 4244)
//    std::transform(command.begin(), command.end(), command.begin(), ::toupper);
//#pragma warning(pop)

    // only handle commands containing ".vacdm"
    //if (0 != command.find(".VACDM")) return false;

    // master command
    //if (std::string::npos != command.find("MASTER")) {

        /*DisplayMessage("Cannot upgrade to Master");
        DisplayMessage(userIsNotEligibleMessage);
        return;*/
    /*} else if (std::string::npos != command.find("SLAVE")) {
        DisplayMessage("Executing vACDM as the SLAVE");
        Logger::instance().log(Logger::LogSender::vACDM, "Switched to SLAVE", Logger::LogLevel::Info);
        com::Server::instance().setMaster(false);

        // Clear all pilot data when switching to slave mode
        DataManager::instance().clearAllPilotData();
        DisplayMessage("All pilot data cleared");

        return true;
    } else if (std::string::npos != command.find("RELOAD")) {
        this->reloadConfiguration();
        return true;
    } else if (std::string::npos != command.find("LOG")) {
        if (std::string::npos != command.find("LOGLEVEL")) {
            DisplayMessage(Logger::instance().handleLogLevelCommand(command));
        } else {
            DisplayMessage(Logger::instance().handleLogCommand(command));
        }
        return true;
    } else if (std::string::npos != command.find("UPDATERATE")) {
        const auto elements = vacdm::utils::String::splitString(command, " ");
        if (elements.size() != 3) {
            DisplayMessage("Usage: .vacdm UPDATERATE value");
            return true;
        }
        if (false == isNumber(elements[2]) ||
            std::stoi(elements[2]) < minUpdateCycleSeconds && std::stoi(elements[2]) > maxUpdateCycleSeconds) {
            DisplayMessage("Usage: .vacdm UPDATERATE value");
            DisplayMessage("Value must be number between " + std::to_string(minUpdateCycleSeconds) + " and " +
                           std::to_string(maxUpdateCycleSeconds));
            return true;
        }

        DisplayMessage(DataManager::instance().setUpdateCycleSeconds(std::stoi(elements[2])));

        return true;
    }*/
}  // namespace vacdm