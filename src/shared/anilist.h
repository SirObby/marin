#pragma once

#include <dpp/dpp.h>
#include <dpp/json.h>
#include <string>

namespace anilist {
    dpp::task<nlohmann::json> query(dpp::cluster& bot, const std::string& query_str, const nlohmann::json& variables);
}
