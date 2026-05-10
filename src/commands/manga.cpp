#include "manga.h"
#include "../shared/anilist.h"
#include "../shared/render.h"
#include "../shared/database.h"
#include "../shared/i18n.h"
#include <format>
#include <regex>

namespace commands {
    dpp::task<void> manga(dpp::slashcommand_t event) {
        dpp::cluster* bot = event.from()->creator;
        dpp::async thinking = event.co_thinking(false);
        std::string locale = Database::get().get_user_locale(event.command.usr.id);

        int search_id = 0;
        std::string search_term = "";
        
        if (std::holds_alternative<int64_t>(event.get_parameter("id"))) {
            search_id = std::get<int64_t>(event.get_parameter("id"));
        } else if (std::holds_alternative<std::string>(event.get_parameter("name"))) {
            search_term = std::get<std::string>(event.get_parameter("name"));
        }

        nlohmann::json response;

        if (search_id > 0) {
            std::string query_str = R"(
            query ($id: Int) {
              Media(id: $id, type: MANGA) {
                id title { romaji english native } description coverImage { extraLarge large }
                genres chapters averageScore isAdult status type format
              }
            }
            )";
            nlohmann::json vars;
            vars["id"] = search_id;
            response = co_await anilist::query(*bot, query_str, vars);
        } else {
            std::string query_str = R"(
            query ($search: String) {
              Page(page: 1, perPage: 10) {
                media(search: $search, type: MANGA) {
                  id title { romaji english native } description coverImage { extraLarge large }
                  genres chapters averageScore isAdult status type format
                }
              }
            }
            )";
            nlohmann::json vars;
            vars["search"] = search_term;
            response = co_await anilist::query(*bot, query_str, vars);
        }
        
        co_await thinking;

        if (response.empty() || !response.contains("data")) {
            event.edit_response(I18n::get().t("SEARCH_NOT_FOUND", locale));
            co_return;
        }

        if (search_id > 0) {
            if (response["data"]["Media"].is_null()) {
                event.edit_response(I18n::get().t("SEARCH_NOT_FOUND", locale));
                co_return;
            }
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
            if (event.command.guild_id != 0) emb.set_color(Database::get().get_guild_theme(event.command.guild_id));
            m.add_embed(emb);
            
            dpp::component action_row;
            action_row.set_type(dpp::cot_action_row)
                .add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_success).set_label(I18n::get().t("ADD_FAVORITE", locale)).set_id("fav_MANGA_" + std::to_string(search_id)))
                .add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_secondary).set_label(I18n::get().t("VIEW_RELATED", locale)).set_id("relations_MANGA_" + std::to_string(search_id)))
                .add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_secondary).set_label(I18n::get().t("VIEW_STAFF", locale)).set_id("staff_MANGA_" + std::to_string(search_id)));
            
            m.add_component(action_row);
            event.edit_response(m);
            co_return;
        }

        auto media_list = response["data"]["Page"]["media"];
        if (media_list.empty()) {
            event.edit_response(I18n::get().t("SEARCH_NOT_FOUND", locale));
            co_return;
        }

        if (media_list.size() == 1) {
            auto media = media_list[0];
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
            int id = media["id"].get<int>();
            dpp::message m;
            auto emb = render::media(media);
            if (event.command.guild_id != 0) emb.set_color(Database::get().get_guild_theme(event.command.guild_id));
            m.add_embed(emb);
            
            dpp::component action_row;
            action_row.set_type(dpp::cot_action_row)
                .add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_success).set_label(I18n::get().t("ADD_FAVORITE", locale)).set_id("fav_MANGA_" + std::to_string(id)))
                .add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_secondary).set_label(I18n::get().t("VIEW_RELATED", locale)).set_id("relations_MANGA_" + std::to_string(id)))
                .add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_secondary).set_label(I18n::get().t("VIEW_STAFF", locale)).set_id("staff_MANGA_" + std::to_string(id)));
            
            m.add_component(action_row);
            event.edit_response(m);
        } else {
            dpp::message m(I18n::get().t("MULTIPLE_RESULTS", locale));
            dpp::embed emb = dpp::embed().set_title(I18n::get().t("MULTIPLE_RESULTS", locale));
            if (event.command.guild_id != 0) emb.set_color(Database::get().get_guild_theme(event.command.guild_id));
            else emb.set_color(render::THEME_COLOR);

            dpp::component select_menu;
            select_menu.set_type(dpp::cot_selectmenu).set_placeholder(I18n::get().t("SELECT_PLACEHOLDER", locale)).set_id("manga_select");
            
            std::string desc;
            int index = 1;
            for (const auto& item : media_list) {
                std::string title = render::get_string(item["title"], "romaji");
                if (title.empty()) title = render::get_string(item["title"], "english");
                if (title.empty()) title = render::get_string(item["title"], "native");
                if (title.empty()) title = "Unknown";
                
                std::string summary = render::get_string(item, "description");
                summary = std::regex_replace(summary, std::regex("<[^>]*>"), ""); // Strip HTML
                if (summary.length() > 60) summary = summary.substr(0, 57) + "...";
                
                desc += std::format("**{}.** {} - {}\n", index, title, summary.empty() ? "No description" : summary);
                
                if (title.length() > 100) title = title.substr(0, 97) + "...";
                select_menu.add_select_option(dpp::select_option(title, std::to_string(item["id"].get<int>())));
                index++;
            }
            if (desc.length() > 2000) desc = desc.substr(0, 2000) + "...";
            emb.set_description(desc);
            m.add_embed(emb);
            m.add_component(dpp::component().add_component(select_menu));
            event.edit_response(m);
        }
    }
}
