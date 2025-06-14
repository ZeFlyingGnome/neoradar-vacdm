// NeoVACDM.h
#pragma once
#include <memory>
#include <thread>
#include "SDK.h"
#include "config/PluginConfig.h"
#include "core/NeoVACDMCommandProvider.h"

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

    void DisplayMessage(const std::string &message, const std::string &sender = "");
    void SetGroundState(const Flightplan::Flightplan flightplan, const std::string groundstate);

    // Scope events
    void OnAirportConfigurationsUpdated(const Airport::AirportConfigurationsUpdatedEvent* event) override;
    void OnTimer(int Counter);
    /* void OnFlightPlanFlightPlanDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan) override;
    void OnFlightPlanControllerAssignedDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan, int DataType) override;*/
    void OnTagAction(const Tag::TagActionEvent *event) override;
    void OnTagDropdownAction(const Tag::DropdownActionEvent *event) override;
    void UpdateTagItems();
    /*    bool OnCompileCommand(const char *sCommandLine) override;*/

    // Command handling
    void TagProcessing(const std::string &callsign, const std::string &actionId, const std::string &userInput = "");
    void reloadConfiguration(bool initialLoading = false);

private:
    bool initialized_ = false;
    PluginMetadata metadata_;
    ClientInformation clientInfo_;
    Aircraft::AircraftAPI *aircraftAPI_ = nullptr;
    Airport::AirportAPI *airportAPI_ = nullptr;
    Chat::ChatAPI *chatAPI_ = nullptr;
    Flightplan::FlightplanAPI *flightplanAPI_ = nullptr;
    Fsd::FsdAPI *fsdAPI_ = nullptr;
    PluginSDK::Logger::LoggerAPI *logger_ = nullptr;
    Tag::TagInterface *tagInterface_ = nullptr;

    std::optional<Aircraft::Aircraft> GetAircraftByCallsign(const std::string &callsign);

    std::string m_configFileName = "vacdm.txt";
    PluginConfig m_pluginConfig;
    void changeServerUrl(const std::string &url);

    void runScopeUpdate();
    void checkServerConfiguration();

    void RegisterTagItems();
    void RegisterTagActions();
    void RegisterCommand();

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

    std::string commandId_;

    std::thread m_worker;
    bool m_stop;
    void run();

    std::shared_ptr<NeoVACDMCommandProvider> CommandProvider_;
};

}  // namespace vacdm
