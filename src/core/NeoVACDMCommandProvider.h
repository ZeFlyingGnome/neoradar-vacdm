#pragma once
#include "SDK.h"
#include "NeoVACDM.h"

using namespace PluginSDK;

namespace vacdm {

class NeoVACDM;

class NeoVACDMCommandProvider : public PluginSDK::Chat::CommandProvider
{
public:
    NeoVACDMCommandProvider(vacdm::NeoVACDM *neoVACDM, PluginSDK::Logger::LoggerAPI *logger, Chat::ChatAPI *chatAPI, Fsd::FsdAPI *fsdAPI)
            : neoVACDM_(neoVACDM), logger_(logger), chatAPI_(chatAPI), fsdAPI_(fsdAPI)
            {logger_->info("NeoVACDMCommandProvider constructor");}
		
	Chat::CommandResult Execute(
        const std::string &commandId,
        const std::vector<std::string> &args) override;

private:
    Logger::LoggerAPI *logger_;
    Chat::ChatAPI *chatAPI_ = nullptr;
    Fsd::FsdAPI *fsdAPI_ = nullptr;
    NeoVACDM *neoVACDM_ = nullptr;

};

}  // namespace vacdm