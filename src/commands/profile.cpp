#include "profile.h"
#include "../shared/anilist.h"
#include "../shared/render.h"
#include "../shared/database.h"
#include "../shared/i18n.h"
#include <format>

namespace commands {
    dpp::task<void> profile(dpp::slashcommand_t event) {
        dpp::cluster* bot = event.from()->creator;
        dpp::async thinking = event.co_thinking(false);

        dpp::snowflake user_id;
        if (std::holds_alternative<dpp::snowflake>(event.get_parameter("user"))) {
            user_id = std::get<dpp::snowflake>(event.get_parameter("user"));
        } else {
            user_id = event.command.usr.id;
        }

        std::string locale = Database::get().get_user_locale(event.command.usr.id);

        dpp::user* user = dpp::find_user(user_id);
        std::string username = user ? user->username : std::to_string(user_id);
        std::string avatar_url = user ? user->get_avatar_url() : "";

        auto favs = Database::get().get_favorites(user_id);
        
        dpp::embed emb = dpp::embed()
            .set_title(I18n::get().t("PROFILE_TITLE", locale, {username}));
            
        if (event.command.guild_id != 0) {
            emb.set_color(Database::get().get_guild_theme(event.command.guild_id));
        } else {
            emb.set_color(render::THEME_COLOR);
        }

        if (!avatar_url.empty()) emb.set_thumbnail(avatar_url);

        std::string bio = Database::get().get_user_bio(user_id);
        std::string desc = "";
        if (!bio.empty()) {
            desc += "*" + bio + "*\n\n";
        }
        
        if (favs.empty()) {
            desc += I18n::get().t("PROFILE_EMPTY", locale);
            emb.set_description(desc);
            co_await thinking;
            event.edit_response(dpp::message().add_embed(emb));
            co_return;
        }

        std::string query_str = R"(
        query ($id: Int) {
          Media(id: $id) {
            title { romaji english native }
          }
        }
        )";

        for (const auto& fav : favs) {
            nlohmann::json vars;
            vars["id"] = fav.media_id;
            nlohmann::json response = co_await anilist::query(*bot, query_str, vars);
            if (!response.empty() && response.contains("data") && !response["data"]["Media"].is_null()) {
                auto title_node = response["data"]["Media"]["title"];
                std::string title = render::get_string(title_node, "romaji");
                if (title.empty()) title = render::get_string(title_node, "english");
                if (title.empty()) title = render::get_string(title_node, "native");
                
                std::string rating_str = fav.rating > 0 ? std::format(" (⭐ {}/100)", fav.rating) : "";
                desc += std::format("• [{}]({})\n", title + rating_str, "https://anilist.co/" + fav.type + "/" + std::to_string(fav.media_id));
            }
        }
        
        emb.set_description(desc);

        co_await thinking;
        event.edit_response(dpp::message().add_embed(emb));
    }
}
