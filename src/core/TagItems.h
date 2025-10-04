#pragma once

// #include <wtypes.h>

#include <chrono>
#include <format>
#include <string>

#include "TagItemsColor.h"
#include "core/DataManager.h"
#include "types/Pilot.h"
#include "NeoVACDM.h"


using namespace vacdm::core;
using namespace vacdm::tagitems;

namespace vacdm {
enum itemType {
    EOBT,
    TOBT,
    TSAT,
    TTOT,
    EXOT,
    ASAT,
    AOBT,
    ATOT,
    ASRT,
    AORT,
    CTOT,
    ECFMP_MEASURES,
    EVENT_BOOKING,
};

void NeoVACDM::RegisterTagItems()
{
    PluginSDK::Tag::TagItemDefinition tagDef;

    tagDef.name = "EOBT";
    tagDef.defaultValue = "";
    std::string tagId = tagInterface_->RegisterTagItem(tagDef);
    EOBTTagID_ = tagId;

    tagDef.name = "TOBT";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    TOBTTagID_ = tagId;

    tagDef.name = "TSAT";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    TSATTagID_ = tagId;

    tagDef.name = "TTOT";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    TTOTTagID_ = tagId;

    tagDef.name = "EXOT";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    EXOTTagID_ = tagId;

    tagDef.name = "ASAT";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    ASATTagID_ = tagId;

    tagDef.name = "AOBT";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    AOBTTagID_ = tagId;

    tagDef.name = "ATOT";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    ATOTTagID_ = tagId;

    tagDef.name = "ASRT";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    ASRTTagID_ = tagId;

    tagDef.name = "AORT";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    AORTTagID_ = tagId;

    tagDef.name = "CTOT";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    CTOTTagID_ = tagId;

    tagDef.name = "EventBooking";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    EventBookingTagID_ = tagId;

    tagDef.name = "ECFMPMeasures";
    tagDef.defaultValue = "";
    tagId = tagInterface_->RegisterTagItem(tagDef);
    ECFMPMeasuresTagID_ = tagId;

}

std::string formatTime(const std::chrono::system_clock::time_point timepoint) {
    if (timepoint.time_since_epoch().count() > 0)
        return std::format("{:%H%M}", timepoint);
    else
        return "";
}

void NeoVACDM::UpdateTagItems() {
    std::vector<std::string> callsigns = dataManager_->getPilots();

    for (std::string callsign : callsigns) {

        auto pilot = dataManager_->getPilot(callsign);
        std::string text;
        Tag::TagContext context;
        context.callsign = callsign;

        text = formatTime(pilot.eobt);
        context.colour = Color::colorizeEobt(pilot);
        if (pilot.eobtCache.text != text || pilot.eobtCache.colour != context.colour)
        {
            tagInterface_->UpdateTagValue(EOBTTagID_, text, context);
            dataManager_->setPilotEobtCache(callsign, text, context.colour);
        }

        text = formatTime(pilot.tobt);
        context.colour = Color::colorizeTobt(pilot);
        if (pilot.tobtCache.text != text || pilot.tobtCache.colour != context.colour)
        {
            tagInterface_->UpdateTagValue(TOBTTagID_, text, context);
            dataManager_->setPilotTobtCache(callsign, text, context.colour);
        }

        text = formatTime(pilot.tsat);
        context.colour = Color::colorizeTsat(pilot);
        if (pilot.tsatCache.text != text || pilot.tsatCache.colour != context.colour)
        {
            tagInterface_->UpdateTagValue(TSATTagID_, text, context);
            dataManager_->setPilotTsatCache(callsign, text, context.colour);
        }

        text = formatTime(pilot.ttot);
        context.colour = Color::colorizeTtot(pilot);
        if (pilot.ttotCache.text != text || pilot.ttotCache.colour != context.colour)
        {
            tagInterface_->UpdateTagValue(TTOTTagID_, text, context);
            dataManager_->setPilotTtotCache(callsign, text, context.colour);
        }

        if (pilot.exot.time_since_epoch().count() > 0) {
            text = std::format("{:%M}", pilot.exot);
            context.colour = std::nullopt;
            if (pilot.exotCache.text != text || pilot.exotCache.colour != context.colour)
            {
                tagInterface_->UpdateTagValue(EXOTTagID_, text, context);
                dataManager_->setPilotExotCache(callsign, text, context.colour);
            }
        }

        text = formatTime(pilot.asat);
        context.colour = Color::colorizeAsat(pilot);
        if (pilot.asatCache.text != text || pilot.asatCache.colour != context.colour)
        {
            tagInterface_->UpdateTagValue(ASATTagID_, text, context);
            dataManager_->setPilotAsatCache(callsign, text, context.colour);
        }

        text = formatTime(pilot.aobt);
        context.colour = Color::colorizeAobt(pilot);
        if (pilot.aobtCache.text != text || pilot.aobtCache.colour != context.colour)
        {
            tagInterface_->UpdateTagValue(AOBTTagID_, text, context);
            dataManager_->setPilotAobtCache(callsign, text, context.colour);
        }

        text = formatTime(pilot.atot);
        context.colour = Color::colorizeAtot(pilot);
        if (pilot.atotCache.text != text || pilot.atotCache.colour != context.colour)
        {
            tagInterface_->UpdateTagValue(ATOTTagID_, text, context);
            dataManager_->setPilotAtotCache(callsign, text, context.colour);
        }

        text = formatTime(pilot.asrt);
        context.colour = Color::colorizeAsrt(pilot);
        if (pilot.asrtCache.text != text || pilot.asrtCache.colour != context.colour)
        {
            tagInterface_->UpdateTagValue(ASRTTagID_, text, context);
            dataManager_->setPilotAsrtCache(callsign, text, context.colour);
        }

        text = formatTime(pilot.aort);
        context.colour = Color::colorizeAort(pilot);
        if (pilot.aortCache.text != text || pilot.aortCache.colour != context.colour)
        {
            tagInterface_->UpdateTagValue(AORTTagID_, text, context);
            dataManager_->setPilotAortCache(callsign, text, context.colour);
        }

        text = formatTime(pilot.ctot);
        context.colour = Color::colorizeCtot(pilot);
        if (pilot.ctotCache.text != text || pilot.ctotCache.colour != context.colour)
        {
            tagInterface_->UpdateTagValue(CTOTTagID_, text, context);
            dataManager_->setPilotCtotCache(callsign, text, context.colour);
        }

        if (false == pilot.measures.empty()) {
            const std::int64_t measureMinutes = pilot.measures[0].value / 60;
            const std::int64_t measureSeconds = pilot.measures[0].value % 60;

            text = std::format("{:02}:{:02}", measureMinutes, measureSeconds);
            context.colour = Color::colorizeEcfmpMeasure(pilot);
            if (pilot.ecfmpMeasuresCache.text != text || pilot.ecfmpMeasuresCache.colour != context.colour)
            {
                tagInterface_->UpdateTagValue(ECFMPMeasuresTagID_, text, context);
                dataManager_->setPilotEcfmpMeasuresCache(callsign, text, context.colour);
            }
        }

        text = (pilot.hasBooking ? "B" : "");
        context.colour = Color::colorizeEventBooking(pilot);
        if (pilot.eventBookingCache.text != text || pilot.eventBookingCache.colour != context.colour)
        {
            tagInterface_->UpdateTagValue(EventBookingTagID_, text, context);
            dataManager_->setPilotEventBookingCache(callsign, text, context.colour);
        }

    }
}
}  // namespace vacdm