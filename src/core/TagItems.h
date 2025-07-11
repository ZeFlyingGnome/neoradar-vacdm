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

std::string formatTime(const std::chrono::utc_clock::time_point timepoint) {
    if (timepoint.time_since_epoch().count() > 0)
        return std::format("{:%H%M}", timepoint);
    else
        return "";
}

void NeoVACDM::UpdateTagItems() {
    std::vector<std::string> callsigns = DataManager::instance().getPilots();

    for (std::string callsign : callsigns) {

        auto pilot = DataManager::instance().getPilot(callsign);
        std::string text;
        Tag::TagContext context;
        context.callsign = callsign;

        text = formatTime(pilot.eobt);
        context.colour = Color::colorizeEobt(pilot);
        tagInterface_->UpdateTagValue(EOBTTagID_, text, context);

        text = formatTime(pilot.tobt);
        context.colour = Color::colorizeTobt(pilot);
        tagInterface_->UpdateTagValue(TOBTTagID_, text, context);

        text = formatTime(pilot.tsat);
        context.colour = Color::colorizeTsat(pilot);
        tagInterface_->UpdateTagValue(TSATTagID_, text, context);

        text = formatTime(pilot.ttot);
        context.colour = Color::colorizeTtot(pilot);
        tagInterface_->UpdateTagValue(TTOTTagID_, text, context);

        if (pilot.exot.time_since_epoch().count() > 0) {
            text = std::format("{:%M}", pilot.exot);
            context.colour = std::nullopt;
            tagInterface_->UpdateTagValue(EXOTTagID_, text, context);
        }

        text = formatTime(pilot.asat);
        context.colour = Color::colorizeAsat(pilot);
        tagInterface_->UpdateTagValue(ASATTagID_, text, context);

        text = formatTime(pilot.aobt);
        context.colour = Color::colorizeAobt(pilot);
        tagInterface_->UpdateTagValue(AOBTTagID_, text, context);

        text = formatTime(pilot.atot);
        context.colour = Color::colorizeAtot(pilot);
        tagInterface_->UpdateTagValue(ATOTTagID_, text, context);

        text = formatTime(pilot.asrt);
        context.colour = Color::colorizeAsrt(pilot);
        tagInterface_->UpdateTagValue(ASRTTagID_, text, context);

        text = formatTime(pilot.aort);
        context.colour = Color::colorizeAort(pilot);
        tagInterface_->UpdateTagValue(AORTTagID_, text, context);

        text = formatTime(pilot.ctot);
        context.colour = Color::colorizeCtot(pilot);
        tagInterface_->UpdateTagValue(CTOTTagID_, text, context);

        if (false == pilot.measures.empty()) {
            const std::int64_t measureMinutes = pilot.measures[0].value / 60;
            const std::int64_t measureSeconds = pilot.measures[0].value % 60;

            text = std::format("{:02}:{:02}", measureMinutes, measureSeconds);
            context.colour = Color::colorizeEcfmpMeasure(pilot);
            tagInterface_->UpdateTagValue(ECFMPMeasuresTagID_, text, context);
        }

        text = (pilot.hasBooking ? "B" : "");
        context.colour = Color::colorizeEventBooking(pilot);
        tagInterface_->UpdateTagValue(EventBookingTagID_, text, context);

    }
}
}  // namespace vacdm