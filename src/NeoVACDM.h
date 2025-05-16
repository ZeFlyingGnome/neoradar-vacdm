// NeoVACDM.h
#pragma once
#include <memory>
#include "SDK.h"
# include "config/PluginConfig.h"

using namespace PluginSDK;

namespace vacdm {

class NeoVACDM : public BasePlugin
{
public:
    NeoVACDM();
    ~NeoVACDM();

    void Initialize(const PluginMetadata &metadata, CoreAPI *coreAPI, ClientInformation info) override;
    void Shutdown() override;
    PluginMetadata GetMetadata() const override;

    void DisplayMessage(const std::string &message, const std::string &sender = "vACDM");
    void SetGroundState(const Flightplan::Flightplan flightplan, const std::string groundstate);

    // Scope events
    void OnAirportConfigurationsUpdated(const Airport::AirportConfigurationsUpdatedEvent* event) override;
    /*void OnTimer(int Counter) override;
    void OnFlightPlanFlightPlanDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan) override;
    void OnFlightPlanControllerAssignedDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan, int DataType) override;*/
    void OnTagAction(const Tag::TagActionEvent *event) override;
    void OnTagDropdownAction(const Tag::DropdownActionEvent *event) override;
    /*void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode,
                      int TagData, char sItemString[16], int *pColorCode, COLORREF *pRGB, double *pFontSize) override;
    bool OnCompileCommand(const char *sCommandLine) override;*/
    void OnFlightplanUpdated(const Flightplan::FlightplanUpdatedEvent* event) override;

    // COmmand handling
    void SetMaster();


private:
    bool initialized_ = false;
    PluginMetadata metadata_;
    ClientInformation clientInfo_;
    CoreAPI *coreAPI_ = nullptr;
    // Fsd::FsdAPI *fsdAPI_ = nullptr;
    Aircraft::AircraftAPI *aircraftAPI_ = nullptr;
    Flightplan::FlightplanAPI *flightplanAPI_ = nullptr;
    // ControllerData::ControllerDataAPI *controllerDataAPI_ = nullptr;
    Logger::LoggerAPI *logger_ = nullptr;

    std::optional<Aircraft::Aircraft> GetAircraftByCallsign(const std::string &callsign);

    // !!!!! WINDOWS ONLY !!!!!!!
    std::string m_configFileName = "\\vacdm.txt";
    PluginConfig m_pluginConfig;
    void changeServerUrl(const std::string &url);

    void runScopeUpdate();
    void checkServerConfiguration();
    void reloadConfiguration(bool initialLoading = false);

    void RegisterTagItems();
    void RegisterTagActions();

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
	std::string ResetTOBTConfirmActionId_;
	std::string ResetAORTActionId_;
	std::string ResetAOBTActionId_;
	std::string ResetMenuActionId_;
	std::string ResetPilotActionId_;

    std::string VACDMMasterActionId_;
    
};

}  // namespace vacdm