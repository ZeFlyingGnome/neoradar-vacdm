#include "NeoVACDM.h"

extern "C" PLUGIN_API PluginSDK::BasePlugin *CreatePluginInstance()
{
    try
    {
        return new vacdm::NeoVACDM();
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}