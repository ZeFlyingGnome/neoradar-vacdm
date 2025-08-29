#pragma once

#include <string>
#include <array>
#include <optional>

namespace vacdm {
struct PluginConfig {
    bool valid = true;
    std::string serverUrl = "https://app.vacdm.net";
    int updateCycleSeconds = 5;
    std::array<unsigned int, 3> lightgreen = std::array<unsigned int, 3>({127, 252, 73});
    std::array<unsigned int, 3> lightblue = std::array<unsigned int, 3>({53, 218, 235});
    std::array<unsigned int, 3> green = std::array<unsigned int, 3>({0, 181, 27});
    std::array<unsigned int, 3> blue = std::array<unsigned int, 3>({0, 0, 255});
    std::array<unsigned int, 3> lightyellow = std::array<unsigned int, 3>({255, 255, 191});
    std::array<unsigned int, 3> yellow = std::array<unsigned int, 3>({255, 255, 0});
    std::array<unsigned int, 3> orange = std::array<unsigned int, 3>({255, 153, 0});
    std::array<unsigned int, 3> red = std::array<unsigned int, 3>({255, 0, 0});
    std::array<unsigned int, 3> grey = std::array<unsigned int, 3>({153, 153, 153});
    std::array<unsigned int, 3> white = std::array<unsigned int, 3>({255, 255, 255});
    std::array<unsigned int, 3> debug = std::array<unsigned int, 3>({255, 0, 255});
};
}  // namespace vacdm