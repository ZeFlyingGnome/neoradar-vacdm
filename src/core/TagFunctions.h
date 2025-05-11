#pragma once

#include "core/DataManager.h"
#include "core/Server.h"
#include "types/Pilot.h"
#include "utils/Date.h"
#include "utils/Number.h"
#include "NeoVACDM.h"

using namespace vacdm;
using namespace vacdm::core;
using namespace vacdm::com;

namespace vacdm {

enum itemFunction {
    EXOT_MODIFY = 1,
    EXOT_NEW_VALUE,
    TOBT_NOW,
    TOBT_MANUAL,
    TOBT_MANUAL_EDIT,
    TOBT_MENU,
    ASAT_NOW,
    ASAT_NOW_AND_STARTUP,
    STARTUP_REQUEST,
    TOBT_CONFIRM,
    OFFBLOCK_REQUEST,
    AOBT_NOW_AND_STATE,
    RESET_TOBT,
    RESET_ASAT,
    RESET_ASRT,
    RESET_TOBT_CONFIRM,
    RESET_AORT,
    RESET_AOBT_AND_STATE,
    RESET_MENU,
    RESET_PILOT,
};

void NeoVACDM::RegisterTagActions()
{
    PluginSDK::Tag::TagActionDefinition tagDef;
    tagDef.name = "EXOTModify";
    tagDef.requiresInput = true;
    tagDef.description = "Modify EXOT";
    std::string tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    EXOTModifyActionId_ = tagId;

    tagDef.name = "TOBTNow";
    tagDef.requiresInput = false;
    tagDef.description = "TOBT Now";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    TOBTNowActionId_ = tagId;

    tagDef.name = "TOBTManual";
    tagDef.requiresInput = true;
    tagDef.description = "Set TOBT";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    TOBTManualActionId_ = tagId;

    tagDef.name = "TOBTConfirm";
    tagDef.requiresInput = false;
    tagDef.description = "TOBT confirm";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    TOBTConfirmActionId_ = tagId;    

    tagDef.name = "TOBTMenu";
    tagDef.requiresInput = false;
    tagDef.description = "TOBT menu";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    TOBTMenuActionId_ = tagId;    

    tagDef.name = "ASATNow";
    tagDef.requiresInput = false;
    tagDef.description = "ASAT Now";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    ASATNowActionId_ = tagId;

    tagDef.name = "ASATNowAndStartup";
    tagDef.requiresInput = false;
    tagDef.description = "ASAT now and startup state";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    ASATNowAndStartupActionId_ = tagId;

    tagDef.name = "StartupRequest";
    tagDef.requiresInput = false;
    tagDef.description = "Startup Request";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    StartupRequestActionId_ = tagId;

    tagDef.name = "OffblockRequest";
    tagDef.requiresInput = false;
    tagDef.description = "Request Offblock";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    OffblockRequestActionId_ = tagId;

    tagDef.name = "AOBTNowAndState";
    tagDef.requiresInput = false;
    tagDef.description = "Set AOBT and Groundstate";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    AOBTNowAndStateActionId_ = tagId;

    tagDef.name = "ResetTOBT";
    tagDef.requiresInput = false;
    tagDef.description = "Reset TOBT";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    ResetTOBTActionId_ = tagId;

    tagDef.name = "ResetASAT";
    tagDef.requiresInput = false;
    tagDef.description = "Reset ASAT";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    ResetASATActionId_ = tagId;

    tagDef.name = "ResetTOBTConfirm";
    tagDef.requiresInput = false;
    tagDef.description = "Reset confirmed TOBT";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    ResetTOBTConfirmActionId_ = tagId;

    tagDef.name = "ResetAORT";
    tagDef.requiresInput = false;
    tagDef.description = "Reset Offblock Request";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    ResetAORTActionId_ = tagId;

    tagDef.name = "ResetAOBT";
    tagDef.requiresInput = false;
    tagDef.description = "Reset AOBT";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    ResetAOBTActionId_ = tagId;

    tagDef.name = "ResetMenu";
    tagDef.requiresInput = false;
    tagDef.description = "Reset Menu";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    ResetMenuActionId_ = tagId;

    tagDef.name = "ResetPilot";
    tagDef.requiresInput = false;
    tagDef.description = "Reset TOBT";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    ResetPilotActionId_ = tagId;
   
}

void NeoVACDM::OnTagAction(const PluginSDK::Tag::TagActionEvent *event)
{
    logger_->info("Entering TagActionEvent");
    logger_->info("documentsPath root_name: " + clientInfo_.documentsPath.string());

    if (!initialized_ || !event )
    {
        return;
    }

    logger_->info("NeoVACDM event actionId: " + event->actionId);
    logger_->info("NeoVACDM event tagId: " + event->tagId);
    logger_->info("NeoVACDM event callsign: " + event->callsign);
    logger_->info("NeoVACDM event userInput: " + event->userInput);
}
void NeoVACDM::OnTagDropdownAction(const PluginSDK::Tag::DropdownActionEvent *event)
{
    logger_->info("Entering OnTagDropdownAction");


    if (!initialized_ || !event )
    {
        return;
    }

    logger_->info("NeoVACDM event actionId: " + event->actionId);
    logger_->info("NeoVACDM event componentId: " + event->componentId);
    logger_->info("NeoVACDM event tagId: " + event->tagId);
    logger_->info("NeoVACDM event callsign: " + event->callsign);
}

/*void vACDM::OnFunctionCall(int functionId, const char *itemString, POINT pt, RECT area) {
    std::ignore = pt;

    // do not handle functions if client is not master
    if (false == Server::instance().getMaster()) return;

    auto flightplan = FlightPlanSelectASEL();
    std::string callsign(flightplan.GetCallsign());

    if (false == DataManager::instance().checkPilotExists(callsign)) return;

    auto pilot = DataManager::instance().getPilot(callsign);

    switch (static_cast<itemFunction>(functionId)) {
        case EXOT_MODIFY:
            OpenPopupEdit(area, static_cast<int>(itemFunction::EXOT_NEW_VALUE), itemString);
            break;
        case EXOT_NEW_VALUE:
            if (true == isNumber(itemString)) {
                const auto exot = std::chrono::utc_clock::time_point(std::chrono::minutes(std::atoi(itemString)));
                if (exot != pilot.exot)
                    DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateEXOT, pilot.callsign,
                                                              exot);
            }
            break;
        case TOBT_NOW:
            DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateTOBT, pilot.callsign,
                                                      std::chrono::utc_clock::now());
            break;
        case TOBT_MANUAL:
            OpenPopupEdit(area, TOBT_MANUAL_EDIT, "");
            break;
        case TOBT_MANUAL_EDIT: {
            std::string clock(itemString);
            if (clock.length() == 4 && isNumber(clock)) {
                const auto hours = std::atoi(clock.substr(0, 2).c_str());
                const auto minutes = std::atoi(clock.substr(2, 4).c_str());
                if (hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60)
                    DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateTOBTConfirmed,
                                                              pilot.callsign,
                                                              utils::Date::convertStringToTimePoint(clock));
                else
                    DisplayMessage("Invalid time format. Expected: HHMM (24 hours)");
            } else if (clock.length() != 0) {
                DisplayMessage("Invalid time format. Expected: HHMM (24 hours)");
            }
            break;
        }
        case ASAT_NOW: {
            DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateASAT, pilot.callsign,
                                                      std::chrono::utc_clock::now());
            // if ASRT has not been set yet -> set ASRT
            if (pilot.asrt == types::defaultTime) {
                DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateASRT, pilot.callsign,
                                                          std::chrono::utc_clock::now());
            }
            break;
        }
        case ASAT_NOW_AND_STARTUP: {
            DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateASAT, pilot.callsign,
                                                      std::chrono::utc_clock::now());

            // if ASRT has not been set yet -> set ASRT
            if (pilot.asrt == types::defaultTime) {
                DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateASRT, pilot.callsign,
                                                          std::chrono::utc_clock::now());
            }

            SetGroundState(flightplan, "ST-UP");

            break;
        }
        case STARTUP_REQUEST: {
            DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateASRT, pilot.callsign,
                                                      std::chrono::utc_clock::now());
            break;
        }
        case AOBT_NOW_AND_STATE: {
            // set ASRT if ASRT has not been set yet
            if (pilot.asrt == types::defaultTime) {
                DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateAORT, pilot.callsign,
                                                          std::chrono::utc_clock::now());
            }
            DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateAOBT, pilot.callsign,
                                                      std::chrono::utc_clock::now());

            // set status depending on if the aircraft is positioned at a taxi-out position
            if (pilot.taxizoneIsTaxiout) {
                SetGroundState(flightplan, "TAXI");
            } else {
                SetGroundState(flightplan, "PUSH");
            }
            break;
        }
        case TOBT_CONFIRM: {
            DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateTOBTConfirmed, pilot.callsign,
                                                      pilot.tobt);
            break;
        }
        case OFFBLOCK_REQUEST: {
            DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateAORT, pilot.callsign,
                                                      std::chrono::utc_clock::now());
            break;
        }
        case TOBT_MENU: {
            OpenPopupList(area, "TOBT menu", 1);
            AddPopupListElement("TOBT now", NULL, TOBT_NOW, false, 2, false, false);
            AddPopupListElement("TOBT edit", NULL, TOBT_MANUAL, false, 2, false, false);
            AddPopupListElement("TOBT confirm", NULL, TOBT_CONFIRM, false, 2, false, false);
            break;
        }
        case RESET_TOBT:
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetTOBT, pilot.callsign,
                                                      types::defaultTime);
            break;
        case RESET_ASAT:
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetASAT, pilot.callsign,
                                                      types::defaultTime);
            SetGroundState(flightplan, "NSTS");
            break;
        case RESET_ASRT:
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetASRT, pilot.callsign,
                                                      types::defaultTime);
            break;
        case RESET_TOBT_CONFIRM:
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetTOBTConfirmed, pilot.callsign,
                                                      types::defaultTime);
            break;
        case RESET_AORT:
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetAORT, pilot.callsign,
                                                      types::defaultTime);
            break;
        case RESET_AOBT_AND_STATE:
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetAOBT, pilot.callsign,
                                                      types::defaultTime);
            SetGroundState(flightplan, "NSTS");
            break;
        case RESET_MENU:
            OpenPopupList(area, "RESET menu", 1);
            AddPopupListElement("Reset TOBT", NULL, RESET_TOBT, false, 2, false, false);
            AddPopupListElement("Reset ASAT", NULL, RESET_ASAT, false, 2, false, false);
            AddPopupListElement("Reset ASRT", NULL, RESET_ASRT, false, 2, false, false);
            AddPopupListElement("Reset confirmed TOBT", NULL, RESET_TOBT_CONFIRM, false, 2, false, false);
            AddPopupListElement("Reset AORT", NULL, RESET_AORT, false, 2, false, false);
            AddPopupListElement("Reset AOBT", NULL, RESET_AOBT_AND_STATE, false, 2, false, false);
            AddPopupListElement("Reset Pilot", NULL, RESET_PILOT, false, 2, false, false);
            break;
        case RESET_PILOT:
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetPilot, pilot.callsign,
                                                      types::defaultTime);
            break;
        default:
            break;
    }
}*/
}  // namespace vacdm