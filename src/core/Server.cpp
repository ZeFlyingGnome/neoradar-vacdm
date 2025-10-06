#include "Server.h"

#include <numeric>

#include "Version.h"
#include "utils/Date.h"

using namespace vacdm;
using namespace vacdm::com;
using namespace vacdm::logging;

Server::Server(logging::Logger* vacdmLogger)
    : m_authToken(),
      m_clientMutex(),
      m_client(nullptr),
      m_apiIsChecked(false),
      m_apiIsValid(false),
      m_baseUrl("https://app.vacdm.net"),
      m_clientIsMaster(false),
      m_errorCode(),
      vacdmLogger_(vacdmLogger) {
    initClient();
}

Server::~Server() {
    std::lock_guard guard(m_clientMutex);
    m_client.reset();
}

void Server::initClient() {
    std::lock_guard guard(m_clientMutex);

    // Create a new client with the current base URL
    m_client = std::make_unique<httplib::Client>(m_baseUrl);

    // Configure client settings similar to curl options
    m_client->set_connection_timeout(2);
    m_client->set_read_timeout(5);
    m_client->set_write_timeout(5);

    m_client->enable_server_certificate_verification(false);
    m_client->enable_server_hostname_verification(false);

    // Set default headers
    m_client->set_default_headers({{"Accept", "application/json"}, {"Content-Type", "application/json"},
                                        {"User-Agent", "VACDM Plugin Neo/" + std::string(PLUGIN_VERSION)}});

    // Add authorization if token is available
    if (!m_authToken.empty()) {
        m_client->set_bearer_token_auth(m_authToken);
    }
}

void Server::changeServerAddress(const std::string& url) {
    this->m_baseUrl = url;
    this->m_apiIsChecked = false;
    this->m_apiIsValid = false;

    // Re-initialize client with new URL
    initClient();
}

bool Server::checkWebApi() {
    if (this->m_apiIsChecked == true) return this->m_apiIsValid;

    std::lock_guard guard(m_clientMutex);
    if (!m_client) {
        this->m_apiIsValid = false;
        return m_apiIsValid;
    }

    std::string url = "/api/v1/version";

    // Send GET request
    auto result = m_client->Get(url);
    if (!result || result->status != 200) {
        if (vacdmLogger_)
            vacdmLogger_->log(
                Logger::LogSender::Server,
                "Failed to connect to API: " + (result ? std::to_string(result->status) : "connection error"),
                Logger::LogLevel::Info);
        this->m_apiIsValid = false;
        return m_apiIsValid;
    }

    std::string response = result->body;
    if (vacdmLogger_)
        vacdmLogger_->log(Logger::LogSender::Server, "Received API-version-message: " + response,
                           Logger::LogLevel::Info);
    try {
        auto root = nlohmann::json::parse(response);
        if (PLUGIN_VERSION_MAJOR != root["major"].get<int>()) {
            this->m_errorCode = "Backend-version is incompatible. Please update the plugin.";
            this->m_apiIsValid = false;
        } else {
            this->m_apiIsValid = true;
        }
    } catch (const std::exception& e) {
        if (vacdmLogger_)
            vacdmLogger_->log(Logger::LogSender::Server, "Failed to parse response JSON: " + std::string(e.what()),
                               Logger::LogLevel::Info);
        this->m_errorCode = "Invalid backend-version response: " + response;
        this->m_apiIsValid = false;
    }

    m_apiIsChecked = true;
    return this->m_apiIsValid;
}

Server::ServerConfiguration Server::getServerConfig() {
    if (false == this->m_apiIsChecked || false == this->m_apiIsValid) return Server::ServerConfiguration();

    std::lock_guard guard(m_clientMutex);
    if (m_client) {
        std::string url = "/api/v1/config";
        auto result = m_client->Get(url);

        if (result && result->status == 200) {
            nlohmann::json root;

            if (vacdmLogger_)
                vacdmLogger_->log(Logger::LogSender::Server, "Received configuration: " + result->body,
                                   Logger::LogLevel::Info);

            try {
                root = nlohmann::json::parse(result->body);
                ServerConfiguration_t config;
                config.name = root["serverName"].get<std::string>();
                config.allowMasterInSweatbox = root["allowSimSession"].get<bool>();
                config.allowMasterAsObserver = root["allowObsMaster"].get<bool>();
                return config;
            } catch (const std::exception& e) {
                if (vacdmLogger_)
                    vacdmLogger_->log(Logger::LogSender::Server,
                                       "Failed to parse response JSON: " + std::string(e.what()),
                                       Logger::LogLevel::Info);
            }
        }
    }

    return ServerConfiguration();
}

void Server::retrieveSupportedAirports() {
    if (false == this->m_apiIsChecked || false == this->m_apiIsValid) { return; }

    std::lock_guard guard(m_clientMutex);
    if (m_client) {
        std::string url = "/api/v1/airports";
        auto result = m_client->Get(url);

        if (result && result->status == 200) {
            nlohmann::json root;

            try {
                root = nlohmann::json::parse(result->body);
                std::list<std::string> airports;
                for (const auto& airport : std::as_const(root)) {
                    airports.push_back(airport["icao"].get<std::string>());
                }
                m_supportedAirports = airports;
            } catch (const std::exception& e) {
                if (vacdmLogger_)
                    vacdmLogger_->log(Logger::LogSender::Server,
                                       "Failed to parse response JSON: " + std::string(e.what()),
                                       Logger::LogLevel::Info);
            }
        }
    }
}

std::list<std::string> Server::getSupportedAirports() { return m_supportedAirports; };

std::list<types::Pilot> Server::getPilots(const std::list<std::string> airports) {
    std::lock_guard guard(m_clientMutex);
    if (!m_client) {
        return {};
    }

    std::string url = "/api/v1/pilots";

    // Add airport filter if specified
    if (!airports.empty()) {
        url += "?adep=";
        url += std::accumulate(std::next(airports.begin()), airports.end(), *airports.begin(),
                               [](const std::string& a, const std::string& b) { return a + "," + b; });
    }

    if (vacdmLogger_)
        vacdmLogger_->log(Logger::LogSender::Server, url, Logger::LogLevel::Info);

    auto result = m_client->Get(url);
    if (result && result->status == 200) {
        nlohmann::json root;

        try {
            root = nlohmann::json::parse(result->body);
            std::list<types::Pilot> pilots;

            for (const auto& pilot : std::as_const(root)) {
                pilots.push_back(types::Pilot());

                pilots.back().callsign = pilot["callsign"].get<std::string>();
                pilots.back().lastUpdate = utils::Date::isoStringToTimestamp(pilot["updatedAt"].get<std::string>());
                pilots.back().inactive = pilot["inactive"].get<bool>();

                // position data
                pilots.back().latitude = pilot["position"]["lat"].get<double>();
                pilots.back().longitude = pilot["position"]["lon"].get<double>();
                pilots.back().taxizoneIsTaxiout = pilot["vacdm"]["taxizoneIsTaxiout"].get<bool>();

                // flightplan & clearance data
                pilots.back().origin = pilot["flightplan"]["departure"].get<std::string>();
                pilots.back().destination = pilot["flightplan"]["arrival"].get<std::string>();
                pilots.back().runway = pilot["clearance"]["dep_rwy"].get<std::string>();
                pilots.back().sid = pilot["clearance"]["sid"].get<std::string>();

                // ACDM procedure data
                pilots.back().eobt = utils::Date::isoStringToTimestamp(pilot["vacdm"]["eobt"].get<std::string>());
                pilots.back().tobt = utils::Date::isoStringToTimestamp(pilot["vacdm"]["tobt"].get<std::string>());
                pilots.back().tobt_state = pilot["vacdm"]["tobt_state"].get<std::string>();
                pilots.back().ctot = utils::Date::isoStringToTimestamp(pilot["vacdm"]["ctot"].get<std::string>());
                pilots.back().ttot = utils::Date::isoStringToTimestamp(pilot["vacdm"]["ttot"].get<std::string>());
                pilots.back().tsat = utils::Date::isoStringToTimestamp(pilot["vacdm"]["tsat"].get<std::string>());
                pilots.back().exot =
                    std::chrono::system_clock::time_point(std::chrono::minutes(pilot["vacdm"]["exot"].get<long int>()));
                pilots.back().asat = utils::Date::isoStringToTimestamp(pilot["vacdm"]["asat"].get<std::string>());
                pilots.back().aobt = utils::Date::isoStringToTimestamp(pilot["vacdm"]["aobt"].get<std::string>());
                pilots.back().atot = utils::Date::isoStringToTimestamp(pilot["vacdm"]["atot"].get<std::string>());
                pilots.back().asrt = utils::Date::isoStringToTimestamp(pilot["vacdm"]["asrt"].get<std::string>());
                pilots.back().aort = utils::Date::isoStringToTimestamp(pilot["vacdm"]["aort"].get<std::string>());

                // ECFMP measures
                nlohmann::json measuresArray = pilot["measures"];
                std::vector<types::EcfmpMeasure> parsedMeasures;
                for (const auto& measureObject : std::as_const(measuresArray)) {
                    vacdm::types::EcfmpMeasure measure;

                    measure.ident = measureObject["ident"].get<std::string>();
                    measure.value = measureObject["value"].get<int>();

                    parsedMeasures.push_back(measure);
                }
                pilots.back().measures = parsedMeasures;

                // event booking data
                pilots.back().hasBooking = pilot["hasBooking"].get<bool>();
            }
            if (vacdmLogger_)
                vacdmLogger_->log(Logger::LogSender::Server, "Pilots size: " + std::to_string(pilots.size()),
                                   Logger::LogLevel::Info);
            return pilots;

        } catch (const std::exception& e) {
            if (vacdmLogger_)
                vacdmLogger_->log(Logger::LogSender::Server, "Failed to parse response JSON: " + std::string(e.what()),
                                   Logger::LogLevel::Info);
        }
    }
    return {};
}

void Server::sendPostMessage(const std::string& endpointUrl, const nlohmann::json& root) {
    if (this->m_apiIsChecked == false || this->m_apiIsValid == false || this->m_clientIsMaster == false) return;

    const auto message = root.dump();

    if (root.contains("callsign")) {
        if (vacdmLogger_)
            vacdmLogger_->log(Logger::LogSender::Server,
                               "Posting " + root["callsign"].get<std::string>() + " with message: " + message,
                               Logger::LogLevel::Debug);
    }

    std::lock_guard guard(m_clientMutex);
    if (m_client) {
        auto result = m_client->Post(endpointUrl, message, "application/json");

        if (result && root.contains("callsign")) {
            if (vacdmLogger_)
                vacdmLogger_->log(Logger::LogSender::Server,
                                   "Posted " + root["callsign"].get<std::string>() + " response: " + result->body,
                                   Logger::LogLevel::Debug);
        }
    }
}

void Server::sendPatchMessage(const std::string& endpointUrl, const nlohmann::json& root) {
    if (this->m_apiIsChecked == false || this->m_apiIsValid == false || this->m_clientIsMaster == false) return;

    const auto message = root.dump();

    if (root.contains("callsign")) {
        if (vacdmLogger_)
            vacdmLogger_->log(Logger::LogSender::Server,
                               "Patching " + root["callsign"].get<std::string>() + " with message: " + message,
                               Logger::LogLevel::Debug);
    }

    std::lock_guard guard(m_clientMutex);
    if (m_client) {
        auto result = m_client->Patch(endpointUrl, message, "application/json");

        if (result && root.contains("callsign")) {
            if (vacdmLogger_)
                vacdmLogger_->log(Logger::LogSender::Server,
                                   "Patched " + root["callsign"].get<std::string>() + " response: " + result->body,
                                   Logger::LogLevel::Debug);
        }
    }
}

void Server::sendDeleteMessage(const std::string& endpointUrl) {
    if (this->m_apiIsChecked == false || this->m_apiIsValid == false || this->m_clientIsMaster == false) {
        return;
    }
    std::lock_guard guard(m_clientMutex);
    if (m_client) {
        m_client->Delete(endpointUrl);
    }
}

void Server::postPilot(types::Pilot pilot) {
    nlohmann::json root;

    root["callsign"] = pilot.callsign;
    root["inactive"] = false;

    root["position"] = nlohmann::json();
    root["position"]["lat"] = pilot.latitude;
    root["position"]["lon"] = pilot.longitude;

    root["flightplan"] = nlohmann::json();
    root["flightplan"]["departure"] = pilot.origin;
    root["flightplan"]["arrival"] = pilot.destination;

    root["vacdm"] = nlohmann::json();
    root["vacdm"]["eobt"] = utils::Date::timestampToIsoString(pilot.eobt);
    root["vacdm"]["tobt"] = utils::Date::timestampToIsoString(pilot.tobt);

    root["clearance"] = nlohmann::json();
    root["clearance"]["dep_rwy"] = pilot.runway;
    root["clearance"]["sid"] = pilot.sid;

    this->sendPostMessage("/api/v1/pilots", root);
}

void Server::updateExot(const std::string& callsign, const std::chrono::system_clock::time_point& exot) {
    nlohmann::json root;

    root["callsign"] = callsign;
    root["vacdm"] = nlohmann::json();
    root["vacdm"]["exot"] = std::chrono::duration_cast<std::chrono::minutes>(exot.time_since_epoch()).count();
    root["vacdm"]["tsat"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["ttot"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["asat"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["aobt"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["atot"] = utils::Date::timestampToIsoString(types::defaultTime);

    this->sendPatchMessage("/api/v1/pilots/" + callsign, root);
}

void Server::updateTobt(const types::Pilot& pilot, const std::chrono::system_clock::time_point& tobt, bool manualTobt) {
    nlohmann::json root;

    bool resetTsat = (tobt == types::defaultTime && true == manualTobt) || tobt >= pilot.tsat;
    root["callsign"] = pilot.callsign;
    root["vacdm"] = nlohmann::json();

    root["vacdm"] = nlohmann::json();
    root["vacdm"]["tobt"] = utils::Date::timestampToIsoString(tobt);
    if (true == resetTsat) root["vacdm"]["tsat"] = utils::Date::timestampToIsoString(types::defaultTime);
    if (false == manualTobt) root["vacdm"]["tobt_state"] = "CONFIRMED";

    root["vacdm"]["ttot"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["asat"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["aobt"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["atot"] = utils::Date::timestampToIsoString(types::defaultTime);

    this->sendPatchMessage("/api/v1/pilots/" + pilot.callsign, root);
}

void Server::updateAsat(const std::string& callsign, const std::chrono::system_clock::time_point& asat) {
    nlohmann::json root;

    root["callsign"] = callsign;
    root["vacdm"] = nlohmann::json();
    root["vacdm"]["asat"] = utils::Date::timestampToIsoString(asat);

    this->sendPatchMessage("/api/v1/pilots/" + callsign, root);
}

void Server::updateAsrt(const std::string& callsign, const std::chrono::system_clock::time_point& asrt) {
    nlohmann::json root;

    root["callsign"] = callsign;
    root["vacdm"] = nlohmann::json();
    root["vacdm"]["asrt"] = utils::Date::timestampToIsoString(asrt);

    this->sendPatchMessage("/api/v1/pilots/" + callsign, root);
}

void Server::updateAobt(const std::string& callsign, const std::chrono::system_clock::time_point& aobt) {
    nlohmann::json root;

    root["callsign"] = callsign;
    root["vacdm"] = nlohmann::json();
    root["vacdm"]["aobt"] = utils::Date::timestampToIsoString(aobt);

    this->sendPatchMessage("/api/v1/pilots/" + callsign, root);
}

void Server::updateAort(const std::string& callsign, const std::chrono::system_clock::time_point& aort) {
    nlohmann::json root;

    root["callsign"] = callsign;
    root["vacdm"] = nlohmann::json();
    root["vacdm"]["aort"] = utils::Date::timestampToIsoString(aort);

    this->sendPatchMessage("/api/v1/pilots/" + callsign, root);
}

void Server::resetTobt(const std::string& callsign, const std::chrono::system_clock::time_point& tobt,
                       const std::string& tobtState) {
    nlohmann::json root;

    root["callsign"] = callsign;
    root["vacdm"] = nlohmann::json();
    root["vacdm"]["tobt"] = utils::Date::timestampToIsoString(tobt);
    root["vacdm"]["tobt_state"] = tobtState;
    root["vacdm"]["tsat"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["ttot"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["asat"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["asrt"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["aobt"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["atot"] = utils::Date::timestampToIsoString(types::defaultTime);
    root["vacdm"]["aort"] = utils::Date::timestampToIsoString(types::defaultTime);

    sendPatchMessage("/api/v1/pilots/" + callsign, root);
}

void Server::deletePilot(const std::string& callsign) { this->sendDeleteMessage("/api/v1/pilots/" + callsign); }

void Server::setMaster(bool master) { this->m_clientIsMaster = master; }

bool Server::getMaster() { return this->m_clientIsMaster; }

const std::string& Server::errorMessage() const { return this->m_errorCode; }
