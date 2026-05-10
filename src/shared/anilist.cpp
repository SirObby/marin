#include "anilist.h"
#include <iostream>

namespace anilist {
    dpp::task<nlohmann::json> query(dpp::cluster& bot, const std::string& query_str, const nlohmann::json& variables) {
        nlohmann::json pdata;
        pdata["query"] = query_str;
        if (!variables.empty()) {
            pdata["variables"] = variables;
        }

        std::string postdata = pdata.dump();
        
        dpp::http_request_completion_t http_req = co_await bot.co_request(
            "https://graphql.anilist.co/",
            dpp::m_post,
            postdata,
            "application/json",
            {
                {"Accept", "application/json"}
            }
        );

        if (http_req.status != 200) {
            std::cerr << "AniList API Error: " << http_req.status << "\nBody: " << http_req.body << std::endl;
            if (http_req.body.empty()) {
                co_return nlohmann::json();
            }
        }

        try {
            co_return nlohmann::json::parse(http_req.body);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error from AniList: " << e.what() << std::endl;
            co_return nlohmann::json();
        }
    }
}
