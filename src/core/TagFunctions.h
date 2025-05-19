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

    tagDef.name = "ResetTOBTConfirmed";
    tagDef.requiresInput = false;
    tagDef.description = "Reset confirmed TOBT";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    ResetTOBTConfirmedActionId_ = tagId;

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

    // TOBT Menu
    Tag::DropdownDefinition TOBTdropdown;
    TOBTdropdown.title = "TOBT";
    TOBTdropdown.width = 120;
    TOBTdropdown.maxHeight = 150;

    Tag::DropdownComponent TOBTScrollArea;
    TOBTScrollArea.id = "TOBT_scroll_area";
    TOBTScrollArea.type = Tag::DropdownComponentType::ScrollArea;
    TOBTScrollArea.text = "TOBT";    

    Tag::DropdownComponent TOBTBtn;
    TOBTBtn.id = "TOBTNow";
    TOBTBtn.type = Tag::DropdownComponentType::Button;
    TOBTBtn.text = "TOBT now";
    TOBTScrollArea.children.push_back(TOBTBtn);

    TOBTBtn.id = "TOBTManual";
    TOBTBtn.type = Tag::DropdownComponentType::Button;
    TOBTBtn.text = "Set TOBT";
    TOBTScrollArea.children.push_back(TOBTBtn);

    TOBTBtn.id = "TOBTConfirm";
    TOBTBtn.type = Tag::DropdownComponentType::Button;
    TOBTBtn.text = "TOBT confirm";
    TOBTScrollArea.children.push_back(TOBTBtn);

    TOBTdropdown.components.push_back(TOBTScrollArea);
    coreAPI_->tag().getInterface()->SetActionDropdown(TOBTMenuActionId_, TOBTdropdown);

   // Reset Menu
   Tag::DropdownDefinition resetDropdown;
   resetDropdown.title = "Reset";
   resetDropdown.width = 200;
   resetDropdown.maxHeight = 300;

   Tag::DropdownComponent resetScrollArea;
   resetScrollArea.id = "ResetScrollArea";
   resetScrollArea.type = Tag::DropdownComponentType::ScrollArea;
   resetScrollArea.text = "Reset";    

   Tag::DropdownComponent resetBtn;
   resetBtn.id = "ResetTOBT";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset TOBT";
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetASRT";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset ASRT";
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetASAT";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset ASAT";
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetTOBTConfirmed";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset confirmed TOBT";
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetAORT";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset AORT";
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetAOBTandState";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset AOBT";
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetPilot";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset Pilot";
   resetScrollArea.children.push_back(resetBtn);   

   resetDropdown.components.push_back(resetScrollArea);
   coreAPI_->tag().getInterface()->SetActionDropdown(ResetMenuActionId_, resetDropdown);

   // Waiting for command line
    tagDef.name = "vACDMMaster";
    tagDef.requiresInput = false;
    tagDef.description = "vACD MMaster";
    tagId = coreAPI_->tag().getInterface()->RegisterTagAction(tagDef);
    VACDMMasterActionId_ = tagId;
}

void NeoVACDM::OnTagAction(const PluginSDK::Tag::TagActionEvent *event)
{
    if (!initialized_ || !event )
    {
        return;
    }

    // Handling SetMaster in Tags as command line API is not yet available
    if (event->actionId == "plugin:NeoVACDM:ACTION_SetMaster")
        SetMaster();

    // do not handle functions if client is not master
    if (false == Server::instance().getMaster()) return;

    TagProcessing(event->callsign, event->actionId, event->userInput);
}

void NeoVACDM::OnTagDropdownAction(const PluginSDK::Tag::DropdownActionEvent *event)
{
    if (!initialized_ || !event )
    {
        return;
    }

    // do not handle functions if client is not master
    if (false == Server::instance().getMaster()) return;

    std::string actionId = "plugin:NeoVACDM:ACTION_" + event->componentId;
    TagProcessing(event->callsign, actionId);    
}

void NeoVACDM::TagProcessing(const std::string &callsign, const std::string &actionId, const std::string &userInput)
{
    if (false == DataManager::instance().checkPilotExists(callsign)) return;

    auto pilot = DataManager::instance().getPilot(callsign);

    if (actionId == "plugin:NeoVACDM:ACTION_EXOTModify")
    {
        if (true == isNumber(userInput)) {
            const auto exot = std::chrono::utc_clock::time_point(std::chrono::minutes(std::atoi(userInput.c_str())));
            if (exot != pilot.exot)
                DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateEXOT, pilot.callsign,
                                                            exot);
        }
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_TOBTNow")
    {

            DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateTOBT, pilot.callsign,
                                                      std::chrono::utc_clock::now());
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_TOBTManual") {
        std::string clock(userInput);
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
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ASATNow") {

        DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateASAT, pilot.callsign,
                                                    std::chrono::utc_clock::now());
        // if ASRT has not been set yet -> set ASRT
        if (pilot.asrt == types::defaultTime) {
            DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateASRT, pilot.callsign,
                                                        std::chrono::utc_clock::now());
        }
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ASATNowAndStartup") {
        DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateASAT, pilot.callsign,
                                                    std::chrono::utc_clock::now());

        // if ASRT has not been set yet -> set ASRT
        if (pilot.asrt == types::defaultTime) {
            DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateASRT, pilot.callsign,
                                                        std::chrono::utc_clock::now());
        }

        // SetGroundState(flightplan, "ST-UP"); // Not implemented yet
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_StartupRequest") {
        DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateASRT, pilot.callsign,
                                                    std::chrono::utc_clock::now());
        ;
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_AOBTNowAndState") {
        // set ASRT if ASRT has not been set yet
        if (pilot.asrt == types::defaultTime) {
            DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateAORT, pilot.callsign,
                                                        std::chrono::utc_clock::now());
        }
        DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateAOBT, pilot.callsign,
                                                    std::chrono::utc_clock::now());

        // Not implemented yet
        // set status depending on if the aircraft is positioned at a taxi-out position
        /*if (pilot.taxizoneIsTaxiout) {
            SetGroundState(flightplan, "TAXI");
        } else {
            SetGroundState(flightplan, "PUSH");
        } */
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_TOBTConfirm") {
        DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateTOBTConfirmed, pilot.callsign,
                                                    pilot.tobt);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_OffblockRequest") {
        DataManager::instance().handleTagFunction(DataManager::MessageType::UpdateAORT, pilot.callsign,
                                                    std::chrono::utc_clock::now());
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetTOBT") {
        DataManager::instance().handleTagFunction(DataManager::MessageType::ResetTOBT, pilot.callsign,
                                                    types::defaultTime);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetASAT") {
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetASAT, pilot.callsign,
                                                      types::defaultTime);
            // SetGroundState(flightplan, "NSTS");  // Not implemented yet
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetASRT") {
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetASRT, pilot.callsign,
                                                      types::defaultTime);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetTOBTConfirmed") {
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetTOBTConfirmed, pilot.callsign,
                                                      types::defaultTime);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetAORT") {
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetAORT, pilot.callsign,
                                                      types::defaultTime);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetAOBT") {
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetAOBT, pilot.callsign,
                                                      types::defaultTime);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetAOBTandState") {
            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetAOBT, pilot.callsign,
                                                      types::defaultTime);
            // SetGroundState(flightplan, "NSTS");  // Not yet implemented
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetPilot") {

            DataManager::instance().handleTagFunction(DataManager::MessageType::ResetPilot, pilot.callsign,
                                                      types::defaultTime);
    }
    else
        logger_->info("NeoVACDM TagProcessing: No processing for " + actionId);
}

}  // namespace vacdm