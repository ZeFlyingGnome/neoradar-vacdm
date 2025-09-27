// NeoVACDM.cpp
#include "NeoVACDM.h"
#include <numeric>

#include "Version.h"
#include "config/ConfigParser.h"
#include "core/CompileCommands.h"
#include "core/DataManager.h"
#include "core/Server.h"
#include "core/TagFunctions.h"
#include "core/TagItems.h"
#include "log/Logger.h"
#include "utils/String.h"

using namespace vacdm;
using namespace vacdm::com;
using namespace vacdm::core;
using namespace std::chrono_literals;

NeoVACDM::NeoVACDM() = default;
NeoVACDM::~NeoVACDM() = default;

void NeoVACDM::Initialize(const PluginMetadata &metadata, CoreAPI *coreAPI, ClientInformation info)
{
    metadata_ = metadata;
    clientInfo_ = info;
    CoreAPI *lcoreAPI = coreAPI;
    aircraftAPI_ = &lcoreAPI->aircraft();
    airportAPI_ = &lcoreAPI->airport();
    chatAPI_ = &lcoreAPI->chat();
    controllerDataAPI_ = &lcoreAPI->controllerData();
    flightplanAPI_ = &lcoreAPI->flightplan();
    fsdAPI_ = &lcoreAPI->fsd();
    logger_ = &lcoreAPI->logger();
    tagInterface_ = lcoreAPI->tag().getInterface();

    dataManager_ = std::make_unique<core::DataManager>();

    logging::Logger::instance().setLogger(logger_);


	std::pair<bool, std::string> updateAvailable = newVersionAvailable();
#ifndef DEV    
	if (updateAvailable.first) {
		DisplayMessage("A new version of NeoVACDM is available: " + updateAvailable.second + " (current version: " + PLUGIN_VERSION + ")", false, "Update", true);
	}    
#endif

    DisplayMessage("Version " + std::string(PLUGIN_VERSION) + " loaded", true, "Initialisation");
    logging::Logger::instance().log(logging::Logger::LogSender::vACDM, "Version " + std::string(PLUGIN_VERSION) + " loaded",
                           logging::Logger::LogLevel::System);


    try
    {
        this->RegisterTagItems();
        this->RegisterTagActions();
        this->RegisterCommand();

        this->reloadConfiguration(true);

        initialized_ = true;
    }
    catch (const std::exception &e)
    {
        logger_->error("Failed to initialize NeoVACDM: " + std::string(e.what()));
    }

    this->m_stop = false;
    this->m_worker = std::thread(&NeoVACDM::run, this);
}

std::pair<bool, std::string> NeoVACDM::newVersionAvailable()
{
    httplib::SSLClient cli("api.github.com");
    httplib::Headers headers = { {"User-Agent", "VersionChecker"} };
    std::string apiEndpoint = "/repos/ZeFlyingGnome/neoradar-vacdm/releases/latest";

    auto res = cli.Get(apiEndpoint.c_str(), headers);
    if (res && res->status == 200) {
        try
        {
            auto json = nlohmann::json::parse(res->body);
            std::string latestVersion = json["tag_name"];
            latestVersion.erase(0, 1); // remove leading 'v'
            if (latestVersion != PLUGIN_VERSION) {
                logger_->warning("A new version of NeoVACDM is available: " + latestVersion + " (current version: " + PLUGIN_VERSION + ")");
                return { true, latestVersion };
            }
            else {
                logger_->log(PluginSDK::Logger::LogLevel::Info, "NeoVACDM is up to date.");
                return { false, "" };
            }
        }
        catch (const std::exception& e)
        {
            logger_->error("Failed to parse version information from GitHub: " + std::string(e.what()));
            return { false, "" };
        }
    }
    else {
        logger_->error("Failed to check for NeoVACDM updates. HTTP status: " + std::to_string(res ? res->status : 0));
        return { false, "" };
    }
}

void NeoVACDM::Shutdown()
{
    if (initialized_)
    {
        initialized_ = false;
        logger_->info("NeoVACDM shutdown complete");
    }

    this->m_stop = true;
    this->m_worker.join();

	if (dataManager_) dataManager_.reset();

    this->unRegisterCommand();
}

void NeoVACDM::DisplayMessage(const std::string &message, const bool &dedicated, const std::string &sender, const bool &important) {
    Chat::ClientTextMessageEvent textMessage;
    textMessage.sentFrom = "NeoVACDM";
    (sender.empty()) ? textMessage.message = message : textMessage.message = sender + ": " + message;
    textMessage.useDedicatedChannel = dedicated;
    if (important) {
        textMessage.colour = std::array<int, 3>{255, 0, 0};
    }

    chatAPI_->sendClientMessage(textMessage);
}

void NeoVACDM::checkServerConfiguration() {
    if (Server::instance().checkWebApi() == false) {
        DisplayMessage("Connection failed.", false, "Server");
        DisplayMessage(Server::instance().errorMessage().c_str(), false, "Server");
    } else {
        std::string serverName = Server::instance().getServerConfig().name;
        DisplayMessage(("Connected to " + serverName), true, "Server");
        // set active airports and runways
        this->OnAirportConfigurationsUpdated(nullptr);
    }
}

void NeoVACDM::runScopeUpdate() {
    std::vector<Flightplan::Flightplan> flightplans = flightplanAPI_->getAll();

    for (const auto &flightplan : flightplans)
    {
        auto aircraft = GetAircraftByCallsign(flightplan.callsign);
        // if no aircraft found, skip to next flightplan (prefiles ?)
        if (aircraft) {
            auto distanceFromOrigin = aircraftAPI_->getDistanceFromOrigin(flightplan.callsign);
            if (distanceFromOrigin) {
                dataManager_->queueFlightplanUpdate(flightplan, *aircraft, *distanceFromOrigin);
            }
        }
    }
}

void NeoVACDM::reloadConfiguration(bool initialLoading) {
    PluginConfig newConfig;
    ConfigParser parser;

    std::string settingsPath = clientInfo_.documentsPath.string() + DIR_SEPARATOR + "plugins"+ DIR_SEPARATOR + this->m_configFileName;

    if (false == parser.parse(settingsPath, newConfig) || false == newConfig.valid) {
        std::string message = "vacdm.txt:" + std::to_string(parser.errorLine()) + ": " + parser.errorMessage();
        DisplayMessage(message, false, "Config");
    } else {
        DisplayMessage(true == initialLoading ? "Loaded the config" : "Reloaded the config", true, "Config");
        if (this->m_pluginConfig.serverUrl != newConfig.serverUrl)
            this->changeServerUrl(newConfig.serverUrl);
        else
            this->checkServerConfiguration();

        this->m_pluginConfig = newConfig;
        DisplayMessage(dataManager_->setUpdateCycleSeconds(newConfig.updateCycleSeconds));
        tagitems::Color::updatePluginConfig(newConfig);
    }
}

void NeoVACDM::changeServerUrl(const std::string &url) {
    dataManager_->pause();
    Server::instance().changeServerAddress(url);
    this->checkServerConfiguration();

    dataManager_->resume();
    DisplayMessage("Changed URL to " + url);
    logging::Logger::instance().log(logging::Logger::LogSender::vACDM, "Changed URL to " + url, logging::Logger::LogLevel::Info);
}

void NeoVACDM::OnTimer(int Counter) {
    if (Counter % 5 == 0) this->runScopeUpdate();
}

/* void vACDM::OnFlightPlanFlightPlanDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan) {
    dataManager_->queueFlightplanUpdate(FlightPlan);
}

void vACDM::OnFlightPlanControllerAssignedDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan, int DataType) {
    // preemptive return to only handle relevant changes
    if (EuroScopePlugIn::CTR_DATA_TYPE_SPEED == DataType || EuroScopePlugIn::CTR_DATA_TYPE_MACH == DataType ||
        EuroScopePlugIn::CTR_DATA_TYPE_RATE == DataType || EuroScopePlugIn::CTR_DATA_TYPE_HEADING == DataType ||
        EuroScopePlugIn::CTR_DATA_TYPE_DIRECT_TO == DataType) {
        return;
    }
    dataManager_->queueFlightplanUpdate(FlightPlan);
}*/

void NeoVACDM::OnAirportConfigurationsUpdated(const Airport::AirportConfigurationsUpdatedEvent* event) {

    std::list<std::string> activeAirports;

    std::vector<PluginSDK::Airport::AirportConfig> airportConfigurations = airportAPI_->getConfigurations();
    for (const auto &airportConfiguration : airportConfigurations)
    {
        std::string airportICAO = airportConfiguration.icao;

        // check if the airport has been added already, add if it does not exist
        if (std::find(activeAirports.begin(), activeAirports.end(), airportICAO) == activeAirports.end()) {
            activeAirports.push_back(airportICAO);
        }
    }

    if (activeAirports.empty()) {
        logging::Logger::instance().log(logging::Logger::LogSender::vACDM,
                               "Airport/Runway Change, no active airports: ", logging::Logger::LogLevel::Info);
    } else {
        logging::Logger::instance().log(
            logging::Logger::LogSender::vACDM,
                "Airport/Runway Change, active airports: " +
                std::accumulate(std::next(activeAirports.begin()), activeAirports.end(), activeAirports.front(),
                                [](const std::string &acc, const std::string &str) { return acc + " " + str; }),
            logging::Logger::LogLevel::Info);
    }
    dataManager_->setActiveAirports(activeAirports);
}

void NeoVACDM::run() {
    int counter = 1;
    while (true) {
        counter += 1;
        std::this_thread::sleep_for(1s);

        if (true == this->m_stop) return;
        
        this->OnTimer(counter);

        // Update tags every five seconds
        if (counter % 5 ==0) UpdateTagItems();
    }
    return;
}

PluginSDK::PluginMetadata NeoVACDM::GetMetadata() const
{
    return {"NeoVACDM", PLUGIN_VERSION, "French VACC"};
}

std::optional<Aircraft::Aircraft> NeoVACDM::GetAircraftByCallsign(const std::string &callsign) {
    return aircraftAPI_->getByCallsign(callsign);
    
}  // namespace vacdm