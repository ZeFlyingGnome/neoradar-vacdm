#pragma once

// #include <wtypes.h>

#include <chrono>
#include <format>
#include <string>

#include "TagItemsColor.h"
#include "types/Pilot.h"
#include "NeoVACDM.h"

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
    std::string tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    EOBTTagID_ = tagId;

    tagDef.name = "TOBT";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    TOBTTagID_ = tagId;

    tagDef.name = "TSAT";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    TSATTagID_ = tagId;

    tagDef.name = "TTOT";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    TTOTTagID_ = tagId;

    tagDef.name = "EXOT";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    EXOTTagID_ = tagId;

    tagDef.name = "ASAT";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    ASATTagID_ = tagId;

    tagDef.name = "AOBT";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    AOBTTagID_ = tagId;

    tagDef.name = "ATOT";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    ATOTTagID_ = tagId;

    tagDef.name = "ASRT";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    ASRTTagID_ = tagId;

    tagDef.name = "AORT";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    AORTTagID_ = tagId;

    tagDef.name = "CTOT";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    CTOTTagID_ = tagId;

    tagDef.name = "EventBooking";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
    EventBookingTagID_ = tagId;

    tagDef.name = "ECFMPMeasures";
    tagDef.defaultValue = "";
    tagId = coreAPI_->tag().getInterface()->RegisterTagItem(tagDef);
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
        std::optional<std::array<unsigned int, 3>> color;
        Tag::TagContext context;
        context.callsign = callsign;

        text = formatTime(pilot.eobt);
        context.colour = Color::colorizeEobt(pilot);
        coreAPI_->tag().getInterface()->UpdateTagValue(EOBTTagID_, text, context);

        text = formatTime(pilot.tobt);
        context.colour = Color::colorizeTobt(pilot);
        coreAPI_->tag().getInterface()->UpdateTagValue(TOBTTagID_, text, context);

        text = formatTime(pilot.tsat);
        context.colour = Color::colorizeTsat(pilot);
        coreAPI_->tag().getInterface()->UpdateTagValue(TSATTagID_, text, context);

        text = formatTime(pilot.ttot);
        context.colour = Color::colorizeTtot(pilot);
        coreAPI_->tag().getInterface()->UpdateTagValue(TTOTTagID_, text, context);

        if (pilot.exot.time_since_epoch().count() > 0) {
            text = std::format("{:%M}", pilot.exot);
            context.colour = std::nullopt;
            coreAPI_->tag().getInterface()->UpdateTagValue(EXOTTagID_, text, context);
        }

        text = formatTime(pilot.asat);
        context.colour = Color::colorizeAsat(pilot);
        coreAPI_->tag().getInterface()->UpdateTagValue(ASATTagID_, text, context);

        text = formatTime(pilot.aobt);
        context.colour = Color::colorizeAobt(pilot);
        coreAPI_->tag().getInterface()->UpdateTagValue(AOBTTagID_, text, context);

        text = formatTime(pilot.atot);
        context.colour = Color::colorizeAtot(pilot);
        coreAPI_->tag().getInterface()->UpdateTagValue(ATOTTagID_, text, context);

        text = formatTime(pilot.asrt);
        context.colour = Color::colorizeAsrt(pilot);
        coreAPI_->tag().getInterface()->UpdateTagValue(ASRTTagID_, text, context);

        text = formatTime(pilot.aort);
        context.colour = Color::colorizeAort(pilot);
        coreAPI_->tag().getInterface()->UpdateTagValue(AORTTagID_, text, context);

        text = formatTime(pilot.ctot);
        context.colour = Color::colorizeCtot(pilot);
        coreAPI_->tag().getInterface()->UpdateTagValue(CTOTTagID_, text, context);

        if (false == pilot.measures.empty()) {
            const std::int64_t measureMinutes = pilot.measures[0].value / 60;
            const std::int64_t measureSeconds = pilot.measures[0].value % 60;

            text = std::format("{:02}:{:02}", measureMinutes, measureSeconds);
            context.colour = Color::colorizeEcfmpMeasure(pilot);
            coreAPI_->tag().getInterface()->UpdateTagValue(ECFMPMeasuresTagID_, text, context);
        }

        text = (pilot.hasBooking ? "B" : "");
        context.colour = Color::colorizeEventBooking(pilot);
        coreAPI_->tag().getInterface()->UpdateTagValue(EventBookingTagID_, text, context);

    }
}
}  // namespace vacdm