#pragma once

#include <dpp/dpp.h>
#include <dpp/json.h>

namespace render {
    inline constexpr uint32_t THEME_COLOR = 0xe7c4ca;
    std::string get_string(const nlohmann::json& node, const std::string& key, const std::string& def = "");
    std::string get_nested_string(const nlohmann::json& node, const std::string& key1, const std::string& key2, const std::string& def = "");
    dpp::embed media(const nlohmann::json& node);
    dpp::embed character(const nlohmann::json& node);
    dpp::embed staff(const nlohmann::json& node);
    dpp::embed studio(const nlohmann::json& node);
}
