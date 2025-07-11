#include "DataManager.h"

#include "core/Server.h"
#include "log/Logger.h"
#include "utils/Date.h"

using namespace vacdm::com;
using namespace vacdm::core;
using namespace vacdm::logging;
using namespace std::chrono_literals;
using namespace PluginSDK::Aircraft;
using namespace PluginSDK::Flightplan;

static constexpr std::size_t ConsolidatedData = 0;
static constexpr std::size_t ScopeData = 1;
static constexpr std::size_t ServerData = 2;

DataManager::DataManager() : m_pause(false), m_stop(false) { this->m_worker = std::thread(&DataManager::run, this); }

DataManager::~DataManager() {
    this->m_stop = true;
    this->m_worker.join();
}

DataManager& DataManager::instance() {
    static DataManager __instance;
    return __instance;
}

bool DataManager::checkPilotExists(const std::string& callsign) {
    if (true == this->m_pause) return false;

    std::lock_guard guard(this->m_pilotLock);
    return this->m_pilots.cend() != this->m_pilots.find(callsign);
}

const types::Pilot DataManager::getPilot(const std::string& callsign) {
    std::lock_guard guard(this->m_pilotLock);
    return this->m_pilots.find(callsign)->second[ConsolidatedData];
}

std::vector<std::string> DataManager::getPilots() {
    this->m_pilotLock.lock();
    auto pilotMaps = this->m_pilots;
    this->m_pilotLock.unlock();
    
    std::vector<std::string> pilots;

    for (auto& pilotMap : pilotMaps)
        pilots.push_back(pilotMap.second[ConsolidatedData].callsign);

    return pilots;
}


void DataManager::pause() { this->m_pause = true; }

void DataManager::resume() { this->m_pause = false; }

void DataManager::clearAllPilotData() {
    std::lock_guard guard(this->m_pilotLock);
    this->m_pilots.clear();

    // Also clear any pending updates
    std::lock_guard guardUpdates(this->m_scopeUpdatesLock);
    this->m_scopeFlightplanUpdates.clear();

    // Clear any pending messages
    std::lock_guard guardMessages(this->m_asyncMessagesLock);
    this->m_asynchronousMessages.clear();

    Logger::instance().log(Logger::LogSender::DataManager, "All pilot data cleared", Logger::LogLevel::Info);
}

std::string DataManager::setUpdateCycleSeconds(const int newUpdateCycleSeconds) {
    if (newUpdateCycleSeconds < minUpdateCycleSeconds || newUpdateCycleSeconds > maxUpdateCycleSeconds)
        return "Could not set update rate";

    this->updateCycleSeconds = newUpdateCycleSeconds;

    return "vACDM updating every " +
           (newUpdateCycleSeconds == 1 ? "second" : std::to_string(newUpdateCycleSeconds) + " seconds");
}

void DataManager::run() {
    std::size_t counter = 1;
    while (true) {
        std::this_thread::sleep_for(1s);
        if (true == this->m_stop) return;
        if (true == this->m_pause) continue;

        // run every updateCycleSeconds seconds
        if (counter++ % updateCycleSeconds != 0) continue;

        // obtain a copy of the pilot data, work with the copy to minimize lock time
        this->m_pilotLock.lock();
        auto pilots = this->m_pilots;
        this->m_pilotLock.unlock();

        this->processAsynchronousMessages(pilots);

        this->processScopeUpdates(pilots);

        this->consolidateWithBackend(pilots);

        if (true == Server::instance().getMaster()) {
            std::list<std::tuple<types::Pilot, DataManager::MessageType, nlohmann::json>> transmissionBuffer;
            for (auto& pilot : pilots) {
                nlohmann::json message;
                const auto sendType = DataManager::deltaScopeToBackend(pilot.second, message);
                if (MessageType::None != sendType)
                    transmissionBuffer.push_back({pilot.second[ConsolidatedData], sendType, message});
            }

            for (const auto& transmission : std::as_const(transmissionBuffer)) {
                if (std::get<1>(transmission) == MessageType::Post)
                    com::Server::instance().postPilot(std::get<0>(transmission));
                else if (std::get<1>(transmission) == MessageType::Patch)
                    com::Server::instance().sendPatchMessage("/api/v1/pilots/" + std::get<0>(transmission).callsign,
                                                             std::get<2>(transmission));
            }
        }

        // replace the pilot data with the updated copy
        this->m_pilotLock.lock();
        this->m_pilots = pilots;
        this->m_pilotLock.unlock();
    }
}

void DataManager::processAsynchronousMessages(std::map<std::string, std::array<types::Pilot, 3U>>& pilots) {
    this->m_asyncMessagesLock.lock();
    auto messages = this->m_asynchronousMessages;
    this->m_asynchronousMessages.clear();
    this->m_asyncMessagesLock.unlock();

    for (auto& message : messages) {
        // find pilot in list
        for (auto& [callsign, data] : pilots) {
            if (callsign == message.callsign) {
                std::string messageType;

                switch (message.type) {
                    case MessageType::UpdateEXOT:
                        Server::instance().updateExot(message.callsign, message.value);
                        messageType = "EXOT";
                        break;
                    case MessageType::UpdateTOBT:
                        Server::instance().updateTobt(data[ConsolidatedData], message.value, false);
                        messageType = "TOBT";
                        break;
                    case MessageType::UpdateTOBTConfirmed:
                        Server::instance().updateTobt(data[ConsolidatedData], message.value, true);
                        messageType = "TOBT Confirmed Status";
                        break;
                    case MessageType::UpdateASAT:
                        Server::instance().updateAsat(message.callsign, message.value);
                        messageType = "ASAT";
                        break;
                    case MessageType::UpdateASRT:
                        Server::instance().updateAsrt(message.callsign, message.value);
                        messageType = "ASRT";
                        break;
                    case MessageType::UpdateAOBT:
                        Server::instance().updateAobt(message.callsign, message.value);
                        messageType = "AOBT";
                        break;
                    case MessageType::UpdateAORT:
                        Server::instance().updateAort(message.callsign, message.value);
                        messageType = "AORT";
                        break;
                    case MessageType::ResetTOBT:
                        Server::instance().resetTobt(message.callsign, types::defaultTime,
                                                     data[ConsolidatedData].tobt_state);
                        messageType = "TOBT reset";
                        break;
                    case MessageType::ResetASAT:
                        Server::instance().updateAsat(message.callsign, message.value);
                        messageType = "ASAT reset";
                        break;
                    case MessageType::ResetASRT:
                        Server::instance().updateAsrt(message.callsign, message.value);
                        messageType = "ASRT reset";
                        break;
                    case MessageType::ResetTOBTConfirmed:
                        Server::instance().resetTobt(message.callsign, data[ConsolidatedData].tobt, "GUESS");
                        messageType = "TOBT confirmed reset";
                        break;
                    case MessageType::ResetAORT:
                        Server::instance().updateAort(message.callsign, message.value);
                        messageType = "AORT reset";
                        break;
                    case MessageType::ResetAOBT:
                        Server::instance().updateAobt(message.callsign, message.value);
                        messageType = "AOBT reset";
                        break;
                    case MessageType::ResetPilot:
                        Server::instance().deletePilot(message.callsign);
                        messageType = "Pilot reset";
                        break;

                    default:
                        break;
                }

                Logger::instance().log(Logger::LogSender::DataManager,
                                       "Sending " + messageType + " update: " + callsign + " - " +
                                           utils::Date::timestampToIsoString(message.value),
                                       Logger::LogLevel::Info);

                break;
            }
        }
    }
}

void DataManager::handleTagFunction(MessageType type, const std::string callsign,
                                    const std::chrono::utc_clock::time_point value) {
    // do not handle the tag function if the aircraft does not exist or the client is not master
    if (false == this->checkPilotExists(callsign) || false == Server::instance().getMaster()) return;

    // queue the update message which will be sent to the backend
    {
        std::lock_guard guard(this->m_asyncMessagesLock);
        this->m_asynchronousMessages.push_back({type, callsign, value});
    }

    // set the data locally, gives feedback to user that the action was handled, might get overwritten again in the
    // update cycle if the backend does not accept the message
    std::lock_guard guard(this->m_pilotLock);
    auto it = this->m_pilots.find(callsign);
    auto& pilot = it->second[ConsolidatedData];

    pilot.lastUpdate = std::chrono::utc_clock::now();

    switch (type) {
        case MessageType::UpdateEXOT:
            pilot.exot = value;
            pilot.tsat = types::defaultTime;
            pilot.ttot = types::defaultTime;
            pilot.asat = types::defaultTime;
            pilot.aobt = types::defaultTime;
            pilot.atot = types::defaultTime;
            break;
        case MessageType::UpdateTOBT: {
            bool resetTsat = value >= pilot.tsat;

            pilot.tobt = value;
            if (true == resetTsat) pilot.tsat = types::defaultTime;
            pilot.ttot = types::defaultTime;
            pilot.exot = types::defaultTime;
            pilot.asat = types::defaultTime;
            pilot.aobt = types::defaultTime;
            pilot.atot = types::defaultTime;

            break;
        }
        case MessageType::UpdateTOBTConfirmed: {
            bool resetTsat = value == types::defaultTime || value >= pilot.tsat;

            pilot.tobt = value;
            if (true == resetTsat) pilot.tsat = types::defaultTime;
            pilot.ttot = types::defaultTime;
            pilot.exot = types::defaultTime;
            pilot.asat = types::defaultTime;
            pilot.aobt = types::defaultTime;
            pilot.atot = types::defaultTime;

            break;
        }
        case MessageType::UpdateASAT:
            pilot.asat = value;
            break;
        case MessageType::UpdateASRT:
            pilot.asrt = value;
            break;
        case MessageType::UpdateAOBT:
            pilot.aobt = value;
            break;
        case MessageType::UpdateAORT:
            pilot.aort = value;
            break;
        case MessageType::ResetTOBT:
            pilot.tobt = types::defaultTime;
            pilot.tsat = types::defaultTime;
            pilot.ttot = types::defaultTime;
            pilot.exot = types::defaultTime;
            pilot.asat = types::defaultTime;
            pilot.asrt = types::defaultTime;
            pilot.aobt = types::defaultTime;
            pilot.aort = types::defaultTime;
            pilot.atot = types::defaultTime;
            break;
        case MessageType::ResetASAT:
            pilot.asat = types::defaultTime;
            break;
        case MessageType::ResetASRT:
            pilot.asrt = types::defaultTime;
            break;
        case MessageType::ResetTOBTConfirmed:
            pilot.tobt_state = "GUESS";
            break;
        case MessageType::ResetAORT:
            pilot.aort = types::defaultTime;
            break;
        case MessageType::ResetAOBT:
            pilot.aobt = types::defaultTime;
            break;
        case MessageType::ResetPilot:
            this->m_pilots.erase(it);
            break;
        default:
            break;
    }
}

DataManager::MessageType DataManager::deltaScopeToBackend(const std::array<types::Pilot, 3>& data,
                                                              nlohmann::json& message) {
    message.clear();

    if (data[ServerData].callsign == "" && data[ScopeData].callsign != "") {
        return DataManager::MessageType::Post;
    } else {
        message["callsign"] = data[ScopeData].callsign;

        int deltaCount = 0;

        // if (data[ScopeData].inactive != data[ServerData].inactive) {
        //     message["inactive"] = data[ScopeData].inactive;
        //     deltaCount += 1;
        // }

        auto lastDelta = deltaCount;
        message["position"] = nlohmann::json();
        if (data[ScopeData].latitude != data[ServerData].latitude) {
            message["position"]["lat"] = data[ScopeData].latitude;
            deltaCount += 1;
        }
        if (data[ScopeData].longitude != data[ServerData].longitude) {
            message["position"]["lon"] = data[ScopeData].longitude;
            deltaCount += 1;
        }
        if (deltaCount == lastDelta) message.erase("position");

        // patch flightplan data
        lastDelta = deltaCount;
        message["flightplan"] = nlohmann::json();
        if (data[ScopeData].origin != data[ServerData].origin) {
            deltaCount += 1;
            message["flightplan"]["departure"] = data[ScopeData].origin;
        }
        if (data[ScopeData].destination != data[ServerData].destination) {
            deltaCount += 1;
            message["flightplan"]["arrival"] = data[ScopeData].destination;
        }
        if (deltaCount == lastDelta) message.erase("flightplan");

        // patch clearance data
        lastDelta = deltaCount;
        message["clearance"] = nlohmann::json();
        if (data[ScopeData].runway != data[ServerData].runway) {
            deltaCount += 1;
            message["clearance"]["dep_rwy"] = data[ScopeData].runway;
        }
        if (data[ScopeData].sid != data[ServerData].sid) {
            deltaCount += 1;
            message["clearance"]["sid"] = data[ScopeData].sid;
        }
        if (deltaCount == lastDelta) message.erase("clearance");

        return deltaCount != 0 ? DataManager::MessageType::Patch : DataManager::MessageType::None;
    }
}

void DataManager::setActiveAirports(const std::list<std::string> activeAirports) {
    std::lock_guard guard(this->m_airportLock);
    this->m_activeAirports = activeAirports;
}

void DataManager::queueFlightplanUpdate(Flightplan flightplan, Aircraft aircraft, double distanceFromOrigin) {
    // skip the update if:
    // - the flightplan or its data is invalid
    //  - More than 10nm away from origin
    if (false == flightplan.isValid || distanceFromOrigin > 10.0) {
        return;
    }

    auto pilot = this->CFlightPlanToPilot(flightplan, aircraft, distanceFromOrigin);

    std::lock_guard guard(this->m_scopeUpdatesLock);
    this->m_scopeFlightplanUpdates.push_back({std::chrono::utc_clock::now(), pilot});
}

void DataManager::consolidateWithBackend(std::map<std::string, std::array<types::Pilot, 3U>>& pilots) {
    // retrieving backend data
    auto backendPilots = Server::instance().getPilots(this->m_activeAirports);

    for (auto pilot = pilots.begin(); pilots.end() != pilot;) {
        // update backend data & consolidate
        bool removeFlight = pilot->second[ServerData].inactive == true;
        for (auto updateIt = backendPilots.begin(); updateIt != backendPilots.end(); ++updateIt) {
            if (updateIt->callsign == pilot->second[ScopeData].callsign) {
                Logger::instance().log(
                    Logger::LogSender::DataManager,
                    "Updating " + pilot->second[ScopeData].callsign + " with" + updateIt->callsign,
                    Logger::LogLevel::Info);
                pilot->second[ServerData] = *updateIt;
                DataManager::consolidateData(pilot->second);
                removeFlight = false;
                updateIt = backendPilots.erase(updateIt);
                break;
            }
        }

        // remove pilot if he has been flagged as inactive from the backend
        if (true == removeFlight) {
            pilot = pilots.erase(pilot);
        } else {
            ++pilot;
        }
    }
}

void DataManager::consolidateData(std::array<types::Pilot, 3>& pilot) {
    if (pilot[ScopeData].callsign == pilot[ServerData].callsign) {
        // backend data
        pilot[ConsolidatedData].inactive = pilot[ServerData].inactive;
        pilot[ConsolidatedData].lastUpdate = pilot[ServerData].lastUpdate;

        pilot[ConsolidatedData].eobt = pilot[ServerData].eobt;
        pilot[ConsolidatedData].tobt = pilot[ServerData].tobt;
        pilot[ConsolidatedData].tobt_state = pilot[ServerData].tobt_state;
        pilot[ConsolidatedData].ctot = pilot[ServerData].ctot;
        pilot[ConsolidatedData].ttot = pilot[ServerData].ttot;
        pilot[ConsolidatedData].tsat = pilot[ServerData].tsat;
        pilot[ConsolidatedData].exot = pilot[ServerData].exot;
        pilot[ConsolidatedData].asat = pilot[ServerData].asat;
        pilot[ConsolidatedData].aobt = pilot[ServerData].aobt;
        pilot[ConsolidatedData].atot = pilot[ServerData].atot;
        pilot[ConsolidatedData].asrt = pilot[ServerData].asrt;
        pilot[ConsolidatedData].aort = pilot[ServerData].aort;

        pilot[ConsolidatedData].measures = pilot[ServerData].measures;
        pilot[ConsolidatedData].hasBooking = pilot[ServerData].hasBooking;
        pilot[ConsolidatedData].taxizoneIsTaxiout = pilot[ServerData].taxizoneIsTaxiout;

        // Scope data
        pilot[ConsolidatedData].latitude = pilot[ScopeData].latitude;
        pilot[ConsolidatedData].longitude = pilot[ScopeData].longitude;

        pilot[ConsolidatedData].origin = pilot[ScopeData].origin;
        pilot[ConsolidatedData].destination = pilot[ScopeData].destination;
        pilot[ConsolidatedData].runway = pilot[ScopeData].runway;
        pilot[ConsolidatedData].sid = pilot[ScopeData].sid;

        logging::Logger::instance().log(Logger::LogSender::DataManager, "Consolidated " + pilot[ServerData].callsign,
                                        logging::Logger::LogLevel::Info);
    } else {
        logging::Logger::instance().log(Logger::LogSender::DataManager,
                                        "Callsign mismatch during consolidation: " + pilot[ScopeData].callsign +
                                            ", " + pilot[ServerData].callsign,
                                        logging::Logger::LogLevel::Critical);
    }
}

void DataManager::processScopeUpdates(std::map<std::string, std::array<types::Pilot, 3U>>& pilots) {
    // obtain a copy of the flightplan updates, clear the update list, consolidate flightplan updates
    this->m_scopeUpdatesLock.lock();
    auto flightplanUpdates = this->m_scopeFlightplanUpdates;
    this->m_scopeFlightplanUpdates.clear();
    this->m_scopeUpdatesLock.unlock();

    this->consolidateFlightplanUpdates(flightplanUpdates);

    for (auto& update : flightplanUpdates) {
        bool found = false;

        const auto pilot = update.data;

        // find pilot in list
        for (auto& pair : pilots) {
            if (pilot.callsign == pair.first) {
                Logger::instance().log(Logger::LogSender::DataManager, "Updated data of " + pilot.callsign,
                                       Logger::LogLevel::Info);

                pair.second[ScopeData] = pilot;
                found = true;
                break;
            }
        }

        if (false == found) {
            Logger::instance().log(Logger::LogSender::DataManager, "Added " + pilot.callsign, Logger::LogLevel::Info);
            pilots.insert({pilot.callsign, {pilot, pilot, types::Pilot()}});
        }
    }
}

void DataManager::consolidateFlightplanUpdates(std::list<ScopeFlightplanUpdate>& inputList) {
    std::list<DataManager::ScopeFlightplanUpdate> resultList;

    for (const auto& currentUpdate : inputList) {
        auto pilot = currentUpdate.data;

        // only handle updates for active airports
        {
            std::lock_guard guard(this->m_airportLock);
            bool flightDepartsFromActiveAirport = std::find(m_activeAirports.begin(), m_activeAirports.end(),
                                                            std::string(pilot.origin)) != m_activeAirports.end();
            if (false == flightDepartsFromActiveAirport) continue;
        }

        // Check if the flight plan already exists in the result list
        auto it = std::find_if(resultList.begin(), resultList.end(),
                               [&currentUpdate](const ScopeFlightplanUpdate& existingUpdate) {
                                   return existingUpdate.data.callsign == currentUpdate.data.callsign;
                               });

        if (it != resultList.end()) {
            // Flight plan with the same callsign exists
            // Check if the timeIssued is newer
            if (currentUpdate.timeIssued > it->timeIssued) {
                // Update with the newer data
                *it = currentUpdate;
                Logger::instance().log(Logger::LogSender::DataManager,
                                       "Updated: " + std::string(currentUpdate.data.callsign), Logger::LogLevel::Info);
            } else {
                // Existing data is already newer, no update needed
                Logger::instance().log(Logger::LogSender::DataManager,
                                       "Skipped old update for: " + std::string(currentUpdate.data.callsign),
                                       Logger::LogLevel::Info);
            }
        } else {
            // Flight plan with the callsign doesn't exist, add it to the result list
            resultList.push_back(currentUpdate);
            Logger::instance().log(Logger::LogSender::DataManager,
                                   "Update added: " + std::string(currentUpdate.data.callsign), Logger::LogLevel::Info);
        }
    }

    inputList = resultList;
}

types::Pilot DataManager::CFlightPlanToPilot(const PluginSDK::Flightplan::Flightplan flightplan, const PluginSDK::Aircraft::Aircraft aircraft, double distanceFromOrigin) {
    types::Pilot pilot;

    pilot.callsign = flightplan.callsign;
    pilot.lastUpdate = std::chrono::utc_clock::now();

    // position data
    pilot.latitude = aircraft.position.latitude;
    pilot.longitude = aircraft.position.longitude;
    pilot.trueAltitude = aircraft.position.trueAltitude;
    // pilot.distanceFromOrigin = flightplan.GetDistanceFromOrigin();
    // pilot.isSimulated = flightplan.GetSimulated();
    pilot.distanceFromOrigin = distanceFromOrigin;
    pilot.isSimulated = false;

    // flightplan & clearance data
    pilot.origin = flightplan.origin;
    pilot.destination = flightplan.destination;
    // pilot.runway = flightplan.GetFlightPlanData().GetDepartureRwy();
    // pilot.sid = flightplan.GetFlightPlanData().GetSidName();
    pilot.runway = "08L";
    pilot.sid = "LGL6H";

    // acdm data
    pilot.eobt = utils::Date::convertDepartureTime(flightplan);
    pilot.tobt = pilot.eobt;

    return pilot;
}