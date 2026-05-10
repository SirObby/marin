#include "listeners.h"
#include "../shared/anilist.h"
#include "../shared/render.h"
#include "../shared/database.h"
#include "../shared/i18n.h"
#include <format>

using json = nlohmann::json;

namespace listeners {
    dpp::task<void> on_button_click(const dpp::button_click_t& event) {
        dpp::cluster* bot = event.from()->creator;
        std::string custom_id = event.custom_id;
        std::string locale = Database::get().get_user_locale(event.command.usr.id);

        if (custom_id.starts_with("fav_")) {
            // Open a modal to ask for a rating
            std::string type = custom_id.substr(4, 5); // ANIME or MANGA
            std::string id_str = custom_id.substr(10);
            
            dpp::interaction_modal_response modal("modal_rate_" + type + "_" + id_str, "Rate this (Optional)");
            modal.add_component(
                dpp::component().set_label("Rating (1-100)").set_id("rating_input").set_type(dpp::cot_text).set_placeholder("e.g. 85").set_min_length(0).set_max_length(3).set_text_style(dpp::text_short).set_required(false)
            );
            event.dialog(modal);
            co_return; // Dialogs don't need thinking
        } 
        
        dpp::async thinking = event.co_thinking(true);
        
        if (custom_id.starts_with("staff_")) {
            uint64_t id = std::stoull(custom_id.substr(12));
            std::string query_str = R"(
            query ($id: Int) { Media(id: $id) { staff { edges { role node { name { full } } } } } }
            )";
            nlohmann::json vars; vars["id"] = id;
            json response = co_await anilist::query(*bot, query_str, vars);
            co_await thinking;
            if (!response.empty() && response.contains("data") && !response["data"]["Media"]["staff"]["edges"].empty()) {
                dpp::embed emb = dpp::embed().set_title("Staff");
                if (event.command.guild_id != 0) emb.set_color(Database::get().get_guild_theme(event.command.guild_id));
                std::string desc;
                for (const auto& edge : response["data"]["Media"]["staff"]["edges"]) {
                    std::string role = render::get_string(edge, "role");
                    std::string name = render::get_nested_string(edge, "node", "name");
                    if (edge.contains("node") && edge["node"].contains("name") && edge["node"]["name"].contains("full")) {
                        name = edge["node"]["name"]["full"].get<std::string>();
                    }
                    desc += std::format("**{}**: {}\n", role, name);
                }
                if (desc.length() > 2000) desc = desc.substr(0, 2000) + "...";
                emb.set_description(desc);
                event.edit_response(dpp::message().add_embed(emb));
            } else {
                event.edit_response("No staff found.");
            }
        } else if (custom_id.starts_with("studio_")) {
            uint64_t id = std::stoull(custom_id.substr(13));
            std::string query_str = R"(
            query ($id: Int) { Media(id: $id) { studios { edges { isMain node { name } } } } }
            )";
            nlohmann::json vars; vars["id"] = id;
            json response = co_await anilist::query(*bot, query_str, vars);
            co_await thinking;
            if (!response.empty() && response.contains("data") && !response["data"]["Media"]["studios"]["edges"].empty()) {
                dpp::embed emb = dpp::embed().set_title("Studios");
                if (event.command.guild_id != 0) emb.set_color(Database::get().get_guild_theme(event.command.guild_id));
                std::string desc;
                for (const auto& edge : response["data"]["Media"]["studios"]["edges"]) {
                    bool isMain = edge.contains("isMain") && edge["isMain"].is_boolean() && edge["isMain"].get<bool>();
                    std::string name = render::get_nested_string(edge, "node", "name");
                    desc += std::format("{} {}\n", isMain ? "🌟" : "•", name);
                }
                emb.set_description(desc);
                event.edit_response(dpp::message().add_embed(emb));
            } else {
                event.edit_response("No studios found.");
            }
        } else if (custom_id.starts_with("episodes_")) {
            uint64_t id = std::stoull(custom_id.substr(custom_id.find_last_of('_') + 1));
            std::string query_str = R"(
            query ($id: Int) { 
              Media(id: $id) { 
                idMal
                streamingEpisodes { title url } 
                externalLinks { url site }
                episodes
              } 
            }
            )";
            nlohmann::json vars; vars["id"] = id;
            json response = co_await anilist::query(*bot, query_str, vars);
            
            if (!response.empty() && response.contains("data")) {
                auto media = response["data"]["Media"];
                dpp::embed emb = dpp::embed().set_title("Episodes / Streaming Links");
                if (event.command.guild_id != 0) emb.set_color(Database::get().get_guild_theme(event.command.guild_id));
                std::string desc;

                // Attempt to fetch full episode list from Jikan if idMal exists
                if (media.contains("idMal") && !media["idMal"].is_null()) {
                    int malId = media["idMal"].get<int>();
                    dpp::http_request_completion_t jikan_res = co_await bot->co_request(std::format("https://api.jikan.moe/v4/anime/{}/episodes", malId), dpp::m_get);
                    if (jikan_res.status == 200) {
                        try {
                            json jikan_json = json::parse(jikan_res.body);
                            if (jikan_json.contains("data") && !jikan_json["data"].empty()) {
                                desc += "**Full Episode List (via MyAnimeList):**\n";
                                for (const auto& ep : jikan_json["data"]) {
                                    int epNum = ep["mal_id"].get<int>();
                                    std::string epTitle = ep["title"].get<std::string>();
                                    desc += std::format("`{}.` {}\n", epNum, epTitle);
                                    if (desc.length() > 1800) break; // Hard limit for embed description
                                }
                            }
                        } catch(...) {}
                    }
                }
                
                if (!media["streamingEpisodes"].empty() && desc.length() < 1800) {
                    if (!desc.empty()) desc += "\n";
                    desc += "**Streaming Episodes (AniList):**\n";
                    for (const auto& ep : media["streamingEpisodes"]) {
                        std::string title = render::get_string(ep, "title");
                        std::string url = render::get_string(ep, "url");
                        desc += std::format("• [{}]({})\n", title, url);
                        if (desc.length() > 1950) break;
                    }
                }
                
                if (!media["externalLinks"].empty() && desc.length() < 1900) {
                    if (!desc.empty()) desc += "\n";
                    desc += "**Streaming Sites:**\n";
                    for (const auto& link : media["externalLinks"]) {
                        std::string site = render::get_string(link, "site");
                        std::string url = render::get_string(link, "url");
                        desc += std::format("• [{}]({})\n", site, url);
                        if (desc.length() > 1980) break;
                    }
                }

                if (desc.empty()) {
                    if (!media["episodes"].is_null()) {
                        desc = std::format("This anime has {} episodes, but no direct titles were found.", media["episodes"].get<int>());
                    } else {
                        desc = "No episode information found.";
                    }
                }

                co_await thinking;
                if (desc.length() > 4000) desc = desc.substr(0, 4000) + "..."; // safety
                emb.set_description(desc);
                event.edit_response(dpp::message().add_embed(emb));
            } else {
                co_await thinking;
                event.edit_response("Failed to fetch episode info.");
            }
        } else if (custom_id.starts_with("relations_")) {
            uint64_t id = std::stoull(custom_id.substr(custom_id.find_last_of('_') + 1));
            std::string query_str = R"(
            query ($id: Int) { 
              Media(id: $id) { 
                relations { 
                  edges { 
                    relationType 
                    node { id title { romaji english } type format status } 
                  } 
                } 
              } 
            }
            )";
            nlohmann::json vars; vars["id"] = id;
            json response = co_await anilist::query(*bot, query_str, vars);
            co_await thinking;
            if (!response.empty() && response.contains("data") && !response["data"]["Media"]["relations"]["edges"].empty()) {
                dpp::embed emb = dpp::embed().set_title("Related Works");
                if (event.command.guild_id != 0) emb.set_color(Database::get().get_guild_theme(event.command.guild_id));
                
                dpp::component select_menu;
                select_menu.set_type(dpp::cot_selectmenu).set_placeholder("Select a related work to view...").set_id("anime_select");
                
                std::string desc;
                int count = 0;
                for (const auto& edge : response["data"]["Media"]["relations"]["edges"]) {
                    std::string relType = render::get_string(edge, "relationType");
                    auto node = edge["node"];
                    std::string title = render::get_nested_string(node, "title", "romaji");
                    if (title.empty()) title = render::get_nested_string(node, "title", "english");
                    
                    std::string format = render::get_string(node, "format");
                    std::string status = render::get_string(node, "status");
                    std::string type = render::get_string(node, "type");
                    uint64_t rid = node["id"].get<uint64_t>();

                    desc += std::format("**{}**: {} ({}, {})\n", relType, title, format, status);
                    
                    if (count < 25) {
                        std::string label = title;
                        if (label.length() > 100) label = label.substr(0, 97) + "...";
                        select_menu.add_select_option(dpp::select_option(label, std::to_string(rid)).set_description(relType + " - " + format));
                        count++;
                    }
                }
                emb.set_description(desc);
                dpp::message m;
                m.add_embed(emb);
                if (count > 0) m.add_component(dpp::component().add_component(select_menu));
                event.edit_response(m);
            } else {
                event.edit_response("No related works found.");
            }
        }
        co_return;
    }
}
