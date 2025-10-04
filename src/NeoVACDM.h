// NeoVACDM.h
#pragma once
#include <map>
#include <memory>
#include <thread>

#include <NeoRadarSDK/SDK.h>

#include "config/PluginConfig.h"
#include "core/DataManager.h"
#include "core/NeoVACDMCommandProvider.h"
#include "core/Server.h"

using namespace PluginSDK;

namespace vacdm {

class NeoVACDMCommandProvider;

class NeoVACDM : public BasePlugin
{
public:
    NeoVACDM();
    ~NeoVACDM();

    void Initialize(const PluginMetadata &metadata, CoreAPI *coreAPI, ClientInformation info) override;
    void Shutdown() override;
    PluginMetadata GetMetadata() const override;

    void DisplayMessage(const std::string &message, const bool &dedicated = true, const std::string &sender = "", const bool &important = false);

    // Scope events
    void OnAirportConfigurationsUpdated(const Airport::AirportConfigurationsUpdatedEvent* event) override;
    void OnTimer(int Counter);
    void OnTagAction(const Tag::TagActionEvent *event) override;
    void OnTagDropdownAction(const Tag::DropdownActionEvent *event) override;
    void UpdateTagItems();

    // Command handling
    void TagProcessing(const std::string &callsign, const std::string &actionId, std::optional<std::string> userInput = std::nullopt);
    void reloadConfiguration(bool initialLoading = false);

    core::DataManager* GetDataManager() const { return dataManager_.get(); }
    com::Server* GetServer() const { return server_.get(); }
    logging::Logger* GetLogger() const { return vacdmLogger_.get(); }

    std::pair<bool, std::string> newVersionAvailable();    

    std::string helpCommandId_;
    std::string masterCommandId_;
    std::string slaveCommandId_;
    std::string reloadCommandId_;
    std::string logCommandId_;
    std::string loglevelCommandId_;
    std::string updaterateCommandId_;    

private:
    bool initialized_ = false;
    PluginMetadata metadata_;
    ClientInformation clientInfo_;
    Aircraft::AircraftAPI *aircraftAPI_ = nullptr;
    Airport::AirportAPI *airportAPI_ = nullptr;
    Chat::ChatAPI *chatAPI_ = nullptr;
    ControllerData::ControllerDataAPI *controllerDataAPI_ = nullptr;
    Flightplan::FlightplanAPI *flightplanAPI_ = nullptr;
    Fsd::FsdAPI *fsdAPI_ = nullptr;
    PluginSDK::Logger::LoggerAPI *logger_ = nullptr;
    Tag::TagInterface *tagInterface_ = nullptr;

    std::unique_ptr<core::DataManager> dataManager_ = nullptr;
    std::unique_ptr<com::Server> server_ = nullptr;
    std::unique_ptr<logging::Logger> vacdmLogger_ = nullptr;

    std::optional<Aircraft::Aircraft> GetAircraftByCallsign(const std::string &callsign);

    std::string m_configFileName = "vacdm.txt";
    PluginConfig m_pluginConfig;
    void changeServerUrl(const std::string &url);

    void runScopeUpdate();
    void checkServerConfiguration();

    void RegisterTagItems();
    void RegisterTagActions();
    void RegisterCommand();
    void unRegisterCommand();

    // IDs
    std::string EOBTTagID_;
    std::string TOBTTagID_;
    std::string TSATTagID_;
    std::string TTOTTagID_;
    std::string EXOTTagID_;
    std::string ASATTagID_;
    std::string AOBTTagID_;
    std::string ATOTTagID_;
    std::string ASRTTagID_;
    std::string AORTTagID_;
    std::string CTOTTagID_;
    std::string EventBookingTagID_;
    std::string ECFMPMeasuresTagID_;

    std::string EXOTModifyActionId_;
    std::string TOBTNowActionId_;
    std::string TOBTManualActionId_;
    std::string TOBTConfirmActionId_;
    std::string TOBTMenuActionId_;
    std::string ASATNowActionId_;
    std::string ASATNowAndStartupActionId_;
    std::string StartupRequestActionId_;
    std::string OffblockRequestActionId_;
    std::string AOBTNowAndStateActionId_;
	
	std::string ResetTOBTActionId_;
	std::string ResetASATActionId_;
	std::string ResetTOBTConfirmedActionId_;
	std::string ResetAORTActionId_;
	std::string ResetAOBTActionId_;
	std::string ResetMenuActionId_;
	std::string ResetPilotActionId_;
 
    std::thread m_worker;
    bool m_stop;
    void run();

    std::shared_ptr<NeoVACDMCommandProvider> CommandProvider_;
};

}  // namespace vacdm
