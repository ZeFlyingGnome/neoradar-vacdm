#pragma once

#include <list>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <NeoRadarSDK/SDK.h>

#include <nlohmann/json.hpp>

#include "log/Logger.h"
#include "Server.h"
#include "types/Pilot.h"

using namespace vacdm;

namespace vacdm::core {

constexpr int maxUpdateCycleSeconds = 10;
constexpr int minUpdateCycleSeconds = 1;
class DataManager {
   public:
    DataManager(com::Server* server, logging::Logger* vacdmLogger);
    ~DataManager();

    std::string setUpdateCycleSeconds(const int newUpdateCycleSeconds);

    enum class MessageType {
        None,
        Post,
        Patch,
        UpdateEXOT,
        UpdateTOBT,
        UpdateTOBTConfirmed,
        UpdateASAT,
        UpdateASRT,
        UpdateAOBT,
        UpdateAORT,
        ResetTOBT,
        ResetASAT,
        ResetASRT,
        ResetTOBTConfirmed,
        ResetAORT,
        ResetAOBT,
        ResetPilot
    };

   private:
    std::thread m_worker;
    bool m_pause;
    bool m_stop;

    com::Server* server_ = nullptr;
    logging::Logger* vacdmLogger_ = nullptr;

    void run();
    
    int updateCycleSeconds = 5;
    std::mutex m_pilotLock;
    std::map<std::string, std::array<types::Pilot, 3>> m_pilots;
    std::mutex m_airportLock;
    std::list<std::string> m_activeAirports;

    struct ScopeFlightplanUpdate {
        std::chrono::system_clock::time_point timeIssued;
        types::Pilot data;
    };

    std::mutex m_scopeUpdatesLock;
    std::list<ScopeFlightplanUpdate> m_scopeFlightplanUpdates;

    /// @brief consolidates all flightplan updates by throwing out old updates and keeping the most current ones
    /// @param list of flightplans to consolidate
    void consolidateFlightplanUpdates(std::list<ScopeFlightplanUpdate> &list);
    /// @brief updates the pilots with the saved Scope flightplan updates
    /// @param pilots to update
    void processScopeUpdates(std::map<std::string, std::array<types::Pilot, 3U>> &pilots);
    /// @brief gathers all information from Flightplan and Aircraft and converts it to type Pilot
    types::Pilot CFlightPlanToPilot(const PluginSDK::Flightplan::Flightplan flightplan, const PluginSDK::Aircraft::Aircraft aircraft, double distanceFromOrigin);
    /// @brief updates the local data with the data from the backend
    /// @param pilots to update
    void consolidateWithBackend(std::map<std::string, std::array<types::Pilot, 3U>> &pilots);
    /// @brief consolidates Scope and backend data
    /// @param pilot
    void consolidateData(std::array<types::Pilot, 3> &pilot);

    MessageType deltaScopeToBackend(const std::array<types::Pilot, 3> &data, nlohmann::json &message);

    struct AsynchronousMessage {
        const MessageType type;
        const std::string callsign;
        const std::chrono::system_clock::time_point value;
    };

    std::mutex m_asyncMessagesLock;
    std::list<struct AsynchronousMessage> m_asynchronousMessages;
    void processAsynchronousMessages(std::map<std::string, std::array<types::Pilot, 3U>> &pilots);

   public:
    void setActiveAirports(const std::list<std::string> activeAirports);
    void queueFlightplanUpdate(PluginSDK::Flightplan::Flightplan flightplan, PluginSDK::Aircraft::Aircraft aircraft, double distanceFromOrigin);
    void handleTagFunction(MessageType message, const std::string callsign,
                           const std::chrono::system_clock::time_point value);

    bool checkPilotExists(const std::string &callsign);
    const types::Pilot getPilot(const std::string &callsign);
    std::vector<std::string> getPilots();
    void pause();
    void resume();
    void clearAllPilotData();

    void setPilotEobtCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotTobtCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotTsatCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotTtotCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotExotCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotAsatCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotAobtCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotAtotCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotAsrtCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotAortCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotCtotCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotEventBookingCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
    void setPilotEcfmpMeasuresCache(const std::string &callsign, const std::string &text, const std::optional<std::array<unsigned int, 3>> &colour);
};
}  // namespace vacdm::core
