#include <memory>
#pragma warning(push, 0)
#include <EuroScopePlugIn.h>
#pragma warning(pop)

#include "log/SpdLogger.h"
#include "vACDM.h"

std::unique_ptr<EuroScopePlugIn::CPlugIn> Plugin;

void __declspec(dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn **ppPlugInInstance) {
    // Initialize logger first
    vacdm::logging::SpdLogger::initialize();

    // Initialize plugin
    Plugin.reset(new vacdm::vACDM());
    *ppPlugInInstance = Plugin.get();
}

void __declspec(dllexport) EuroScopePlugInExit(void) {
    Plugin.release();
}
