#include "Server.h"

#include <numeric>

#include "Version.h"
#include "log/Logger.h"
#include "utils/Date.h"

using namespace vacdm;
using namespace vacdm::com;
using namespace vacdm::logging;

static std::string __receivedDeleteData;
static std::string __receivedGetData;
static std::string __receivedPatchData;
static std::string __receivedPostData;

static std::size_t receiveCurlDelete(void* ptr, std::size_t size, std::size_t nmemb, void* stream) {
    (void)stream;

    std::string serverResult = static_cast<char*>(ptr);
    __receivedDeleteData += serverResult;
    return size * nmemb;
}

static std::size_t receiveCurlGet(void* ptr, std::size_t size, std::size_t nmemb, void* stream) {
    (void)stream;

    std::size_t realsize = size * nmemb;
    __receivedGetData.append((char*)ptr, realsize);
    return realsize;
}

static std::size_t receiveCurlPatch(void* ptr, std::size_t size, std::size_t nmemb, void* stream) {
    (void)stream;

    std::size_t realsize = size * nmemb;
    __receivedPatchData.append((char*)ptr, realsize);
    return realsize;
}

static std::size_t receiveCurlPost(void* ptr, std::size_t size, std::size_t nmemb, void* stream) {
    (void)stream;

    std::size_t realsize = size * nmemb;
    __receivedPostData.append((char*)ptr, realsize);
    return realsize;
}

Server::Server()
    : m_authToken(),
      m_getRequest(),
      m_postRequest(),
      m_patchRequest(),
      m_deleteRequest(),
      m_apiIsChecked(false),
      m_apiIsValid(false),
      m_baseUrl("https://app.vacdm.net"),
      m_clientIsMaster(false),
      m_errorCode() {
    /* configure the get request */
    curl_easy_setopt(m_getRequest.socket, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(m_getRequest.socket, CURLOPT_SSL_VERIFYHOST, 1L);
    curl_easy_setopt(m_getRequest.socket, CURLOPT_HTTP_VERSION, static_cast<long>(CURL_HTTP_VERSION_1_1));
    curl_easy_setopt(m_getRequest.socket, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(m_getRequest.socket, CURLOPT_WRITEFUNCTION, receiveCurlGet);
    curl_easy_setopt(m_getRequest.socket, CURLOPT_TIMEOUT, 2L);

    /* configure the post request */
    curl_easy_setopt(m_postRequest.socket, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_postRequest.socket, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_postRequest.socket, CURLOPT_HTTP_VERSION, static_cast<long>(CURL_HTTP_VERSION_1_1));
    curl_easy_setopt(m_postRequest.socket, CURLOPT_WRITEFUNCTION, receiveCurlPost);
    curl_easy_setopt(m_postRequest.socket, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(m_postRequest.socket, CURLOPT_VERBOSE, 1);
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + this->m_authToken).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(m_postRequest.socket, CURLOPT_HTTPHEADER, headers);

    /* configure the patch request */
    curl_easy_setopt(m_patchRequest.socket, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_patchRequest.socket, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_patchRequest.socket, CURLOPT_HTTP_VERSION, static_cast<long>(CURL_HTTP_VERSION_1_1));
    curl_easy_setopt(m_patchRequest.socket, CURLOPT_WRITEFUNCTION, receiveCurlPatch);
    curl_easy_setopt(m_patchRequest.socket, CURLOPT_CUSTOMREQUEST, "PATCH");
    curl_easy_setopt(m_patchRequest.socket, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(m_patchRequest.socket, CURLOPT_HTTPHEADER, headers);

    /* configure the delete request */
    curl_easy_setopt(m_deleteRequest.socket, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_deleteRequest.socket, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_deleteRequest.socket, CURLOPT_HTTP_VERSION, static_cast<long>(CURL_HTTP_VERSION_1_1));
    curl_easy_setopt(m_deleteRequest.socket, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(m_deleteRequest.socket, CURLOPT_WRITEFUNCTION, receiveCurlDelete);
    curl_easy_setopt(m_deleteRequest.socket, CURLOPT_TIMEOUT, 2L);
}

Server::~Server() {
    if (nullptr != m_getRequest.socket) {
        std::lock_guard guard(m_getRequest.lock);
        curl_easy_cleanup(m_getRequest.socket);
        m_getRequest.socket = nullptr;
    }

    if (nullptr != m_postRequest.socket) {
        std::lock_guard guard(m_postRequest.lock);
        curl_easy_cleanup(m_postRequest.socket);
        m_postRequest.socket = nullptr;
    }

    if (nullptr != m_patchRequest.socket) {
        std::lock_guard guard(m_patchRequest.lock);
        curl_easy_cleanup(m_patchRequest.socket);
        m_patchRequest.socket = nullptr;
    }

    if (nullptr != m_deleteRequest.socket) {
        std::lock_guard guard(m_deleteRequest.lock);
        curl_easy_cleanup(m_deleteRequest.socket);
        m_deleteRequest.socket = nullptr;
    }
}

void Server::changeServerAddress(const std::string& url) {
    this->m_baseUrl = url;
    this->m_apiIsChecked = false;
    this->m_apiIsValid = false;
}

bool Server::checkWebApi() {
    if (this->m_apiIsChecked == true) return this->m_apiIsValid;

    std::lock_guard guard(m_getRequest.lock);
    if (m_getRequest.socket == nullptr) {
        this->m_apiIsValid = false;
        return m_apiIsValid;
    }

    __receivedGetData.clear();

    std::string url = m_baseUrl + "/api/v1/version";
    curl_easy_setopt(m_getRequest.socket, CURLOPT_URL, url.c_str());

    // send the GET request
    CURLcode result = curl_easy_perform(m_getRequest.socket);
    if (result != CURLE_OK) {
        this->m_apiIsValid = false;
        return m_apiIsValid;
    }

    std::string errors;
    nlohmann::json root;
    Logger::instance().log(Logger::LogSender::Server, "Received API-version-message: " + __receivedGetData,
                           Logger::LogLevel::Info);
    
    
    try
    {
        root = nlohmann::json::parse(__receivedGetData.c_str());
        if ( PLUGIN_VERSION_MAJOR != root["major"].get<int>() ) {
            this->m_errorCode = "Backend-version is incompatible. Please update the plugin.";
            this->m_apiIsValid = false;
        } else {
            this->m_apiIsValid = true;
        }
    }
    catch(const std::exception& e)
    {
        Logger::instance().log(Logger::LogSender::Server, "Failed to parse response JSON: " + std::string(e.what()),
                        Logger::LogLevel::Info);
        this->m_errorCode = "Invalid backend-version response: " + __receivedGetData;
        this->m_apiIsValid = false;
    }
    m_apiIsChecked = true;
    return this->m_apiIsValid;
}

Server::ServerConfiguration Server::getServerConfig() {
    if (false == this->m_apiIsChecked || false == this->m_apiIsValid) return Server::ServerConfiguration();

    std::lock_guard guard(m_getRequest.lock);
    if (nullptr != m_getRequest.socket) {
        __receivedGetData.clear();

        std::string url = m_baseUrl + "/api/v1/config";
        curl_easy_setopt(m_getRequest.socket, CURLOPT_URL, url.c_str());

        /* send the command */
        CURLcode result = curl_easy_perform(m_getRequest.socket);
        if (CURLE_OK == result) {
            std::string errors;
            nlohmann::json root;

            Logger::instance().log(Logger::LogSender::Server, "Received configuration: " + __receivedGetData,
                                   Logger::LogLevel::Info);

            try
            {
                root = nlohmann::json::parse(__receivedGetData.c_str());
                ServerConfiguration_t config;
                config.name = root["serverName"].get<std::string>();
                config.allowMasterInSweatbox = root["allowSimSession"].get<bool>();
                config.allowMasterAsObserver = root["allowObsMaster"].get<bool>();
                return config;
            }
            catch(const std::exception& e)
            {
            Logger::instance().log(Logger::LogSender::Server, "Failed to parse response JSON: " + std::string(e.what()),
                        Logger::LogLevel::Info);
            }
        }
    }

    return ServerConfiguration();
}

std::list<types::Pilot> Server::getPilots(const std::list<std::string> airports) {
    std::lock_guard guard(m_getRequest.lock);
    if (nullptr != m_getRequest.socket) {
        __receivedGetData.clear();

        std::string url = m_baseUrl + "/api/v1/pilots";
        if (airports.size() != 0) {
            url += "?adep=" +
                   std::accumulate(std::next(airports.begin()), airports.end(), airports.front(),
                                   [](const std::string& acc, const std::string& str) { return acc + "&adep=" + str; });
        }
        Logger::instance().log(Logger::LogSender::Server, url, Logger::LogLevel::Info);

        curl_easy_setopt(m_getRequest.socket, CURLOPT_URL, url.c_str());

        // send GET request
        CURLcode result = curl_easy_perform(m_getRequest.socket);
        if (result == CURLE_OK) {
            std::string errors;
            nlohmann::json root;

            // Logger::instance().log(Logger::LogSender::Server, "Received data" + __receivedGetData,
            //                        Logger::LogLevel::Debug);

            try
            {
                root = nlohmann::json::parse(__receivedGetData.c_str());
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
                        std::chrono::utc_clock::time_point(std::chrono::minutes(pilot["vacdm"]["exot"].get<long int>()));
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
                Logger::instance().log(Logger::LogSender::Server, "Pilots size: " + std::to_string(pilots.size()),
                                       Logger::LogLevel::Info);
                return pilots;
            }
            catch(const std::exception& e)
            {
                Logger::instance().log(Logger::LogSender::Server, "Failed to parse response JSON: " + std::string(e.what()),
                        Logger::LogLevel::Info);
            }
        }
    }

    return {};
}

void Server::sendPostMessage(const std::string& endpointUrl, const nlohmann::json& root) {
    if (this->m_apiIsChecked == false || this->m_apiIsValid == false || this->m_clientIsMaster == false) return;

    const auto message = root.dump();

    Logger::instance().log(Logger::LogSender::Server,
                           "Posting " + root["callsign"].get<std::string>() + " with message: " + message,
                           Logger::LogLevel::Debug);

    std::lock_guard guard(this->m_postRequest.lock);
    if (m_postRequest.socket != nullptr) {
        std::string url = m_baseUrl + endpointUrl;
        curl_easy_setopt(m_postRequest.socket, CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_postRequest.socket, CURLOPT_POSTFIELDS, message.c_str());

        curl_easy_perform(m_postRequest.socket);

        Logger::instance().log(Logger::LogSender::Server,
                               "Posted " + root["callsign"].get<std::string>() + " response: " + __receivedPostData,
                               Logger::LogLevel::Debug);
        __receivedPostData.clear();
    }
}

void Server::sendPatchMessage(const std::string& endpointUrl, const nlohmann::json& root) {
    if (this->m_apiIsChecked == false || this->m_apiIsValid == false || this->m_clientIsMaster == false) return;

    const auto message = root.dump();

    Logger::instance().log(Logger::LogSender::Server,
                           "Patching " + root["callsign"].get<std::string>() + " with message: " + message,
                           Logger::LogLevel::Debug);

    std::lock_guard guard(this->m_patchRequest.lock);
    if (m_patchRequest.socket != nullptr) {
        std::string url = m_baseUrl + endpointUrl;
        curl_easy_setopt(m_patchRequest.socket, CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_patchRequest.socket, CURLOPT_POSTFIELDS, message.c_str());

        curl_easy_perform(m_patchRequest.socket);

        Logger::instance().log(Logger::LogSender::Server,
                               "Patched " + root["callsign"].get<std::string>() + " response: " + __receivedPatchData,
                               Logger::LogLevel::Debug);
        __receivedPatchData.clear();
    }
}

void Server::sendDeleteMessage(const std::string& endpointUrl) {
    if (this->m_apiIsChecked == false || this->m_apiIsValid == false || this->m_clientIsMaster == false) return;

    std::lock_guard guard(this->m_deleteRequest.lock);
    if (m_deleteRequest.socket != nullptr) {
        std::string url = m_baseUrl + endpointUrl;

        curl_easy_setopt(m_deleteRequest.socket, CURLOPT_URL, url.c_str());

        curl_easy_perform(m_deleteRequest.socket);
        __receivedDeleteData.clear();
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

void Server::updateExot(const std::string& callsign, const std::chrono::utc_clock::time_point& exot) {
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

void Server::updateTobt(const types::Pilot& pilot, const std::chrono::utc_clock::time_point& tobt, bool manualTobt) {
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

void Server::updateAsat(const std::string& callsign, const std::chrono::utc_clock::time_point& asat) {
    nlohmann::json root;

    root["callsign"] = callsign;
    root["vacdm"] = nlohmann::json();
    root["vacdm"]["asat"] = utils::Date::timestampToIsoString(asat);

    this->sendPatchMessage("/api/v1/pilots/" + callsign, root);
}

void Server::updateAsrt(const std::string& callsign, const std::chrono::utc_clock::time_point& asrt) {
    nlohmann::json root;

    root["callsign"] = callsign;
    root["vacdm"] = nlohmann::json();
    root["vacdm"]["asrt"] = utils::Date::timestampToIsoString(asrt);

    this->sendPatchMessage("/api/v1/pilots/" + callsign, root);
}

void Server::updateAobt(const std::string& callsign, const std::chrono::utc_clock::time_point& aobt) {
    nlohmann::json root;

    root["callsign"] = callsign;
    root["vacdm"] = nlohmann::json();
    root["vacdm"]["aobt"] = utils::Date::timestampToIsoString(aobt);

    this->sendPatchMessage("/api/v1/pilots/" + callsign, root);
}

void Server::updateAort(const std::string& callsign, const std::chrono::utc_clock::time_point& aort) {
    nlohmann::json root;

    root["callsign"] = callsign;
    root["vacdm"] = nlohmann::json();
    root["vacdm"]["aort"] = utils::Date::timestampToIsoString(aort);

    this->sendPatchMessage("/api/v1/pilots/" + callsign, root);
}

void Server::resetTobt(const std::string& callsign, const std::chrono::utc_clock::time_point& tobt,
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

Server& Server::instance() {
    static Server __instance;
    return __instance;
}
