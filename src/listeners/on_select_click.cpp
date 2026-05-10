#include "listeners.h"
#include "../shared/anilist.h"
#include "../shared/render.h"
#include "../shared/database.h"
#include "../shared/i18n.h"

using json = nlohmann::json;

namespace listeners {
    dpp::task<void> on_select_click(const dpp::select_click_t& event) {
        dpp::cluster* bot = event.from()->creator;
        dpp::async thinking = event.co_thinking(false);
        if (event.values.empty()) {
            event.reply(dpp::message("No selection made.").set_flags(dpp::m_ephemeral));
            co_return;
        }
        std::string id_str = event.values[0];
        int id = std::stoi(id_str);
        std::string locale = Database::get().get_user_locale(event.command.usr.id);

        std::string query_str;
        nlohmann::json vars;
        vars["id"] = id;

        if (event.custom_id == "anime_select" || event.custom_id == "manga_select") {
            bool is_anime = (event.custom_id == "anime_select");
            query_str = R"(
            query ($id: Int) {
              Media(id: $id) {
                id title { romaji english native } description coverImage { extraLarge large }
                genres episodes chapters averageScore isAdult status type format idMal
                nextAiringEpisode { airingAt timeUntilAiring episode }
              }
            }
            )";
            json response = co_await anilist::query(*bot, query_str, vars);
            co_await thinking;
            if (!response.empty() && response.contains("data") && !response["data"]["Media"].is_null()) {
                auto media = response["data"]["Media"];
                
                bool is_adult = media.contains("isAdult") && media["isAdult"].is_boolean() && media["isAdult"].get<bool>();
                if (is_adult && event.command.guild_id != 0) {
                    if (!Database::get().get_guild_nsfw(event.command.guild_id)) {
                        event.edit_response(I18n::get().t("NSFW_BLOCKED", locale));
                        co_return;
                    }
                    dpp::channel* c = dpp::find_channel(event.command.channel_id);
                    if (c && !c->is_nsfw()) {
                        event.edit_response(I18n::get().t("NSFW_BLOCKED", locale));
                        co_return;
                    }
                }

                dpp::message m;
                auto emb = render::media(media);
                if (event.command.guild_id != 0) {
                    emb.set_color(Database::get().get_guild_theme(event.command.guild_id));
                }
                m.add_embed(emb);
                
                dpp::component action_row;
                action_row.set_type(dpp::cot_action_row)
                    .add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_success).set_label(I18n::get().t("ADD_FAVORITE", locale)).set_id("fav_" + std::string(is_anime ? "ANIME_" : "MANGA_") + std::to_string(id)));
                
                if (is_anime) {
                    action_row.add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_secondary).set_label(I18n::get().t("VIEW_EPISODES", locale)).set_id("episodes_ANIME_" + std::to_string(id)));
                }
                
                action_row.add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_secondary).set_label(I18n::get().t("VIEW_RELATED", locale)).set_id("relations_" + std::string(is_anime ? "ANIME_" : "MANGA_") + std::to_string(id)));
                
                action_row.add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_secondary).set_label(I18n::get().t("VIEW_STAFF", locale)).set_id("staff_" + std::string(is_anime ? "ANIME_" : "MANGA_") + std::to_string(id)));
                
                if (is_anime) {
                    action_row.add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_secondary).set_label(I18n::get().t("VIEW_STUDIOS", locale)).set_id("studio_ANIME_" + std::to_string(id)));
                }
                
                m.add_component(action_row);
                event.edit_response(m);
            } else {
                event.edit_response("Failed to fetch details.");
            }
        } 
        else if (event.custom_id == "charac_select") {
            query_str = R"(query ($id: Int) { Character(id: $id) { id name { full native } description image { large } gender age } })";
            json response = co_await anilist::query(*bot, query_str, vars);
            co_await thinking;
            if (!response.empty() && response.contains("data") && !response["data"]["Character"].is_null()) {
                auto emb = render::character(response["data"]["Character"]);
                if (event.command.guild_id != 0) emb.set_color(Database::get().get_guild_theme(event.command.guild_id));
                event.edit_response(dpp::message().add_embed(emb));
            } else {
                event.edit_response("Failed to fetch details.");
            }
        }
        co_return;
    }
}
