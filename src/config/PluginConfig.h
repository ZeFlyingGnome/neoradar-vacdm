#pragma once

#include <string>
#include <array>
#include <optional>

namespace vacdm {
struct PluginConfig {
    bool valid = true;
    std::string serverUrl = "https://app.vacdm.net";
    int updateCycleSeconds = 5;
    std::optional<std::array<unsigned int, 3>> lightgreen = std::make_optional<std::array<unsigned int, 3>>({127, 252, 73});
    std::optional<std::array<unsigned int, 3>> lightblue = std::make_optional<std::array<unsigned int, 3>>({53, 218, 235});
    std::optional<std::array<unsigned int, 3>> green = std::make_optional<std::array<unsigned int, 3>>({0, 181, 27});
    std::optional<std::array<unsigned int, 3>> blue = std::make_optional<std::array<unsigned int, 3>>({0, 0, 255});
    std::optional<std::array<unsigned int, 3>> lightyellow = std::make_optional<std::array<unsigned int, 3>>({255, 255, 191});
    std::optional<std::array<unsigned int, 3>> yellow = std::make_optional<std::array<unsigned int, 3>>({255, 255, 0});
    std::optional<std::array<unsigned int, 3>> orange = std::make_optional<std::array<unsigned int, 3>>({255, 153, 0});
    std::optional<std::array<unsigned int, 3>> red = std::make_optional<std::array<unsigned int, 3>>({255, 0, 0});
    std::optional<std::array<unsigned int, 3>> grey = std::make_optional<std::array<unsigned int, 3>>({153, 153, 153});
    std::optional<std::array<unsigned int, 3>> white = std::make_optional<std::array<unsigned int, 3>>({255, 255, 255});
    std::optional<std::array<unsigned int, 3>> debug = std::make_optional<std::array<unsigned int, 3>>({255, 0, 255});
};
}  // namespace vacdm