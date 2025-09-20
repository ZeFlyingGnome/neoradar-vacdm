#include "NeoVACDM.h"

extern "C" PLUGIN_API PluginSDK::BasePlugin *CreatePluginInstance()
{
    return new vacdm::NeoVACDM();
}