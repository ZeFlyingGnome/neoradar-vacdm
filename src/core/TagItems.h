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

/*void vACDM::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget,
                         int ItemCode, int TagData, char sItemString[16], int *pColorCode, COLORREF *pRGB,
                         double *pFontSize) {
    std::ignore = RadarTarget;
    std::ignore = TagData;
    std::ignore = pRGB;
    std::ignore = pFontSize;

    *pColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
    if (nullptr == FlightPlan.GetFlightPlanData().GetPlanType() ||
        0 == std::strlen(FlightPlan.GetFlightPlanData().GetPlanType()))
        return;
    // skip non IFR flights
    if (std::string_view("I") != FlightPlan.GetFlightPlanData().GetPlanType()) {
        return;
    }
    std::string callsign = FlightPlan.GetCallsign();

    if (false == DataManager::instance().checkPilotExists(callsign)) return;

    auto pilot = DataManager::instance().getPilot(callsign);

    std::stringstream outputText;

    switch (static_cast<itemType>(ItemCode)) {
        case itemType::EOBT:
            outputText << formatTime(pilot.eobt);
            *pRGB = Color::colorizeEobt(pilot);
            break;
        case itemType::TOBT:
            outputText << formatTime(pilot.tobt);
            *pRGB = Color::colorizeTobt(pilot);
            break;
        case itemType::TSAT:
            outputText << formatTime(pilot.tsat);
            *pRGB = Color::colorizeTsat(pilot);
            break;
        case itemType::TTOT:
            outputText << formatTime(pilot.ttot);
            *pRGB = Color::colorizeTtot(pilot);
            break;
        case itemType::EXOT:
            if (pilot.exot.time_since_epoch().count() > 0) {
                outputText << std::format("{:%M}", pilot.exot);
                *pColorCode = Color::colorizeExot(pilot);
            }
            break;
        case itemType::ASAT:
            outputText << formatTime(pilot.asat);
            *pRGB = Color::colorizeAsat(pilot);
            break;
        case itemType::AOBT:
            outputText << formatTime(pilot.aobt);
            *pRGB = Color::colorizeAobt(pilot);
            break;
        case itemType::ATOT:
            outputText << formatTime(pilot.atot);
            *pRGB = Color::colorizeAtot(pilot);
            break;
        case itemType::ASRT:
            outputText << formatTime(pilot.asrt);
            *pRGB = Color::colorizeAsrt(pilot);
            break;
        case itemType::AORT:
            outputText << formatTime(pilot.aort);
            *pRGB = Color::colorizeAort(pilot);
            break;
        case itemType::CTOT:
            outputText << formatTime(pilot.ctot);
            *pRGB = Color::colorizeCtot(pilot);
            break;
        case itemType::ECFMP_MEASURES:
            if (false == pilot.measures.empty()) {
                const std::int64_t measureMinutes = pilot.measures[0].value / 60;
                const std::int64_t measureSeconds = pilot.measures[0].value % 60;

                outputText << std::format("{:02}:{:02}", measureMinutes, measureSeconds);
                *pRGB = Color::colorizeEcfmpMeasure(pilot);
            }
            break;
        case itemType::EVENT_BOOKING:
            outputText << (pilot.hasBooking ? "B" : "");
            *pRGB = Color::colorizeEventBooking(pilot);
            break;
        default:
            break;
    }

    std::strcpy(sItemString, outputText.str().c_str());
}*/
}  // namespace vacdm