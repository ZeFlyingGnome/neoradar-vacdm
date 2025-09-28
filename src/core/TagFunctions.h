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
    std::string tagId = tagInterface_->RegisterTagAction(tagDef);
    EXOTModifyActionId_ = tagId;

    tagDef.name = "TOBTNow";
    tagDef.requiresInput = false;
    tagDef.description = "TOBT Now";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    TOBTNowActionId_ = tagId;

    tagDef.name = "TOBTManual";
    tagDef.requiresInput = true;
    tagDef.description = "Set TOBT";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    TOBTManualActionId_ = tagId;

    tagDef.name = "TOBTConfirm";
    tagDef.requiresInput = false;
    tagDef.description = "TOBT confirm";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    TOBTConfirmActionId_ = tagId;    

    tagDef.name = "TOBTMenu";
    tagDef.requiresInput = false;
    tagDef.description = "TOBT menu";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    TOBTMenuActionId_ = tagId;    

    tagDef.name = "ASATNow";
    tagDef.requiresInput = false;
    tagDef.description = "ASAT Now";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    ASATNowActionId_ = tagId;

    tagDef.name = "ASATNowAndStartup";
    tagDef.requiresInput = false;
    tagDef.description = "ASAT now and startup state";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    ASATNowAndStartupActionId_ = tagId;

    tagDef.name = "StartupRequest";
    tagDef.requiresInput = false;
    tagDef.description = "Startup Request";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    StartupRequestActionId_ = tagId;

    tagDef.name = "OffblockRequest";
    tagDef.requiresInput = false;
    tagDef.description = "Request Offblock";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    OffblockRequestActionId_ = tagId;

    tagDef.name = "AOBTNowAndState";
    tagDef.requiresInput = false;
    tagDef.description = "Set AOBT and Groundstate";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    AOBTNowAndStateActionId_ = tagId;

    tagDef.name = "ResetTOBT";
    tagDef.requiresInput = false;
    tagDef.description = "Reset TOBT";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    ResetTOBTActionId_ = tagId;

    tagDef.name = "ResetASAT";
    tagDef.requiresInput = false;
    tagDef.description = "Reset ASAT";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    ResetASATActionId_ = tagId;

    tagDef.name = "ResetTOBTConfirmed";
    tagDef.requiresInput = false;
    tagDef.description = "Reset confirmed TOBT";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    ResetTOBTConfirmedActionId_ = tagId;

    tagDef.name = "ResetAORT";
    tagDef.requiresInput = false;
    tagDef.description = "Reset Offblock Request";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    ResetAORTActionId_ = tagId;

    tagDef.name = "ResetAOBT";
    tagDef.requiresInput = false;
    tagDef.description = "Reset AOBT";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    ResetAOBTActionId_ = tagId;

    tagDef.name = "ResetMenu";
    tagDef.requiresInput = false;
    tagDef.description = "Reset Menu";
    tagId = tagInterface_->RegisterTagAction(tagDef);
    ResetMenuActionId_ = tagId;

    tagDef.name = "ResetPilot";
    tagDef.requiresInput = false;
    tagDef.description = "Reset TOBT";
    tagId = tagInterface_->RegisterTagAction(tagDef);
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
    TOBTBtn.requiresInput = false;
    TOBTScrollArea.children.push_back(TOBTBtn);

    TOBTBtn.id = "TOBTManual";
    TOBTBtn.type = Tag::DropdownComponentType::Button;
    TOBTBtn.text = "Set TOBT";
    TOBTBtn.requiresInput = true;
    TOBTScrollArea.children.push_back(TOBTBtn);

    TOBTBtn.id = "TOBTConfirm";
    TOBTBtn.type = Tag::DropdownComponentType::Button;
    TOBTBtn.text = "TOBT confirm";
    TOBTBtn.requiresInput = false;
    TOBTScrollArea.children.push_back(TOBTBtn);

    TOBTdropdown.components.push_back(TOBTScrollArea);
    tagInterface_->SetActionDropdown(TOBTMenuActionId_, TOBTdropdown);

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
   resetBtn.requiresInput = false;
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetASRT";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset ASRT";
   resetBtn.requiresInput = false;
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetASAT";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset ASAT";
   resetBtn.requiresInput = false;
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetTOBTConfirmed";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset confirmed TOBT";
   resetBtn.requiresInput = false;
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetAORT";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset AORT";
   resetBtn.requiresInput = false;
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetAOBTandState";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset AOBT";
   resetBtn.requiresInput = false;
   resetScrollArea.children.push_back(resetBtn);

   resetBtn.id = "ResetPilot";
   resetBtn.type = Tag::DropdownComponentType::Button;
   resetBtn.text = "Reset Pilot";
   resetBtn.requiresInput = false;
   resetScrollArea.children.push_back(resetBtn);   

   resetDropdown.components.push_back(resetScrollArea);
   tagInterface_->SetActionDropdown(ResetMenuActionId_, resetDropdown);
}

void NeoVACDM::OnTagAction(const PluginSDK::Tag::TagActionEvent *event)
{
    if (!initialized_ || !event )
    {
        return;
    }

    TagProcessing(event->callsign, event->actionId, event->userInput);
}

void NeoVACDM::OnTagDropdownAction(const PluginSDK::Tag::DropdownActionEvent *event)
{
    if (!initialized_ || !event )
    {
        return;
    }

    // do not handle functions if client is not master
    if (!server_) {
#ifdef DEV
        logger_->info("No server instance available 7");
#endif
        return;
    }
    if (false == server_->getMaster()) return;

    std::string actionId = "plugin:NeoVACDM:ACTION_" + event->componentId;
    TagProcessing(event->callsign, actionId, event->userInput);    
}

void NeoVACDM::TagProcessing(const std::string &callsign, const std::string &actionId, std::optional<std::string> userInput)
{
    if (false == dataManager_->checkPilotExists(callsign)) return;

    auto pilot = dataManager_->getPilot(callsign);

    if (actionId == "plugin:NeoVACDM:ACTION_EXOTModify")
    {
        if (userInput && isNumber(*userInput)) {
            const auto exot = std::chrono::system_clock::time_point(std::chrono::minutes(std::atoi(userInput->c_str())));
            if (exot != pilot.exot)
                dataManager_->handleTagFunction(DataManager::MessageType::UpdateEXOT, pilot.callsign,
                                                            exot);
        }
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_TOBTNow")
    {

            dataManager_->handleTagFunction(DataManager::MessageType::UpdateTOBT, pilot.callsign,
                                                      std::chrono::system_clock::now());
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_TOBTManual") {
        if (userInput) {
            std::string clock(*userInput);
            if (clock.length() == 4 && isNumber(clock)) {
                const auto hours = std::atoi(clock.substr(0, 2).c_str());
                const auto minutes = std::atoi(clock.substr(2, 4).c_str());
                if (hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60)
                    dataManager_->handleTagFunction(DataManager::MessageType::UpdateTOBT, pilot.callsign,
                                                                utils::Date::convertStringToTimePoint(clock));
                else
                    DisplayMessage("Invalid time format. Expected: HHMM (24 hours)", false);
            } else if (clock.length() != 0) {
                DisplayMessage("Invalid time format. Expected: HHMM (24 hours)", false);
            }
        } else {
            DisplayMessage("No input provided. Expected: HHMM (24 hours)", false);
        }
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ASATNow") {

        dataManager_->handleTagFunction(DataManager::MessageType::UpdateASAT, pilot.callsign,
                                                    std::chrono::system_clock::now());
        // if ASRT has not been set yet -> set ASRT
        if (pilot.asrt == types::defaultTime) {
            dataManager_->handleTagFunction(DataManager::MessageType::UpdateASRT, pilot.callsign,
                                                        std::chrono::system_clock::now());
        }
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ASATNowAndStartup") {
        dataManager_->handleTagFunction(DataManager::MessageType::UpdateASAT, pilot.callsign,
                                                    std::chrono::system_clock::now());

        // if ASRT has not been set yet -> set ASRT
        if (pilot.asrt == types::defaultTime) {
            dataManager_->handleTagFunction(DataManager::MessageType::UpdateASRT, pilot.callsign,
                                                        std::chrono::system_clock::now());
        }

        controllerDataAPI_->setGroundStatus(pilot.callsign, ControllerData::GroundStatus::Start);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_StartupRequest") {
        dataManager_->handleTagFunction(DataManager::MessageType::UpdateASRT, pilot.callsign,
                                                    std::chrono::system_clock::now());
        ;
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_AOBTNowAndState") {
        // set ASRT if ASRT has not been set yet
        if (pilot.asrt == types::defaultTime) {
            dataManager_->handleTagFunction(DataManager::MessageType::UpdateAORT, pilot.callsign,
                                                        std::chrono::system_clock::now());
        }
        dataManager_->handleTagFunction(DataManager::MessageType::UpdateAOBT, pilot.callsign,
                                                    std::chrono::system_clock::now());

        // set status depending on if the aircraft is positioned at a taxi-out position
        if (pilot.taxizoneIsTaxiout) {
            controllerDataAPI_->setGroundStatus(pilot.callsign, ControllerData::GroundStatus::Taxi);
        } else {
        controllerDataAPI_->setGroundStatus(pilot.callsign, ControllerData::GroundStatus::Push);
        }
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_TOBTConfirm") {
        dataManager_->handleTagFunction(DataManager::MessageType::UpdateTOBTConfirmed, pilot.callsign,
                                                    pilot.tobt);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_OffblockRequest") {
        dataManager_->handleTagFunction(DataManager::MessageType::UpdateAORT, pilot.callsign,
                                                    std::chrono::system_clock::now());
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetTOBT") {
        dataManager_->handleTagFunction(DataManager::MessageType::ResetTOBT, pilot.callsign,
                                                    types::defaultTime);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetASAT") {
            dataManager_->handleTagFunction(DataManager::MessageType::ResetASAT, pilot.callsign,
                                                      types::defaultTime);
            controllerDataAPI_->setGroundStatus(pilot.callsign, ControllerData::GroundStatus::None);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetASRT") {
            dataManager_->handleTagFunction(DataManager::MessageType::ResetASRT, pilot.callsign,
                                                      types::defaultTime);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetTOBTConfirmed") {
            dataManager_->handleTagFunction(DataManager::MessageType::ResetTOBTConfirmed, pilot.callsign,
                                                      types::defaultTime);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetAORT") {
            dataManager_->handleTagFunction(DataManager::MessageType::ResetAORT, pilot.callsign,
                                                      types::defaultTime);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetAOBT") {
            dataManager_->handleTagFunction(DataManager::MessageType::ResetAOBT, pilot.callsign,
                                                      types::defaultTime);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetAOBTandState") {
            dataManager_->handleTagFunction(DataManager::MessageType::ResetAOBT, pilot.callsign,
                                                      types::defaultTime);
            controllerDataAPI_->setGroundStatus(pilot.callsign, ControllerData::GroundStatus::None);
    }
    else if (actionId == "plugin:NeoVACDM:ACTION_ResetPilot") {

            dataManager_->handleTagFunction(DataManager::MessageType::ResetPilot, pilot.callsign,
                                                      types::defaultTime);
    }
    else
        logger_->info("NeoVACDM TagProcessing: No processing for " + actionId);
}

}  // namespace vacdm