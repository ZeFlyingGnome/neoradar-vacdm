#pragma once

#include <string>

namespace vacdm {

struct VACDMCOLOR {
    int r, g, b;
};    

struct PluginConfig {
    bool valid = true;
    std::string serverUrl = "https://app.vacdm.net";
    int updateCycleSeconds = 5;
    VACDMCOLOR lightgreen = {127, 252, 73};
    VACDMCOLOR lightblue = {53, 218, 235};
    VACDMCOLOR green = {0, 181, 27};
    VACDMCOLOR blue = {0, 0, 255};
    VACDMCOLOR lightyellow = {255, 255, 191};
    VACDMCOLOR yellow = {255, 255, 0};
    VACDMCOLOR orange = {255, 153, 0};
    VACDMCOLOR red = {255, 0, 0};
    VACDMCOLOR grey = {153, 153, 153};
    VACDMCOLOR white = {255, 255, 255};
    VACDMCOLOR debug = {255, 0, 255};
};
}  // namespace vacdm