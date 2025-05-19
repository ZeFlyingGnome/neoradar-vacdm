#include "NeoVACDM.h"
#include "log/SpdLogger.h"

extern "C" PLUGIN_API PluginSDK::BasePlugin *CreatePluginInstance()
{

    // Initialize logger first
    vacdm::logging::SpdLogger::initialize();

    try
    {
        return new vacdm::NeoVACDM();
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}
