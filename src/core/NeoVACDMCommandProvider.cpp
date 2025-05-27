#include "NeoVACDMCommandProvider.h"

using namespace PluginSDK;

namespace vacdm {

void NeoVACDMCommandProvider::DisplayMessage(const std::string &message, const std::string &sender) {
    Chat::ClientTextMessageEvent textMessage;
    textMessage.sentFrom = "NeoVACDM";
    textMessage.message = sender + ": " + message;

    chatAPI_->sendClientMessage(textMessage);
}


}  // namespace vacdm