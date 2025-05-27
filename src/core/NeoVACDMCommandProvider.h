#include "SDK.h"

using namespace PluginSDK;

namespace vacdm {

class NeoVACDMCommandProvider : public PluginSDK::Chat::CommandProvider
{
public:
    NeoVACDMCommandProvider(PluginSDK::Logger::LoggerAPI *logger, Chat::ChatAPI *chatAPI)
        : logger_(logger), chatAPI_(chatAPI) {logger_->info("NeoVACDMCommandProvider constructor");}
		
	Chat::CommandResult Execute(
        const std::string &commandId,
        const std::vector<std::string> &args) override;
		
	Chat::ValidationResult ValidateParameters(
        const std::string &commandId,
        const std::vector<std::string> &args,
        bool strict) override;

private:
    void DisplayMessage(const std::string &message, const std::string &sender = "");

    Logger::LoggerAPI *logger_;
    Chat::ChatAPI *chatAPI_ = nullptr;

};

}  // namespace vacdm