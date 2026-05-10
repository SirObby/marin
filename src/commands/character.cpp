#include "character.h"
#include "../shared/anilist.h"
#include "../shared/render.h"
#include <format>

namespace commands {
    dpp::task<void> character(dpp::slashcommand_t event) {
        dpp::cluster* bot = event.from()->creator;
        dpp::async thinking = event.co_thinking(false);

        std::string search_term = std::get<std::string>(event.get_parameter("name"));

        std::string query_str = R"(
        query ($search: String) {
          Page(page: 1, perPage: 10) {
            characters(search: $search) {
              id
              name { full native }
              description
              image { large }
              gender age
            }
          }
        }
        )";

        nlohmann::json vars;
        vars["search"] = search_term;

        nlohmann::json response = co_await anilist::query(*bot, query_str, vars);
        
        co_await thinking;

        if (response.empty() || !response.contains("data") || !response["data"].contains("Page") || response["data"]["Page"]["characters"].empty()) {
            event.edit_response("No character found for that search.");
            co_return;
        }

        auto char_list = response["data"]["Page"]["characters"];
        
        if (char_list.size() == 1) {
            dpp::embed emb = render::character(char_list[0]);
            event.edit_response(dpp::message().add_embed(emb));
        } else {
            dpp::message m("Multiple results found. Please select one:");
            dpp::component select_menu;
            select_menu.set_type(dpp::cot_selectmenu)
                       .set_placeholder("Select a character...")
                       .set_id("charac_select");

            for (const auto& item : char_list) {
                std::string title = render::get_string(item["name"], "full");
                if (title.empty()) title = render::get_string(item["name"], "native");
                if (title.empty()) title = "Unknown";
                
                if (title.length() > 100) title = title.substr(0, 97) + "...";

                select_menu.add_select_option(dpp::select_option(title, std::to_string(item["id"].get<int>())));
            }

            m.add_component(dpp::component().add_component(select_menu));
            event.edit_response(m);
        }
    }
}
