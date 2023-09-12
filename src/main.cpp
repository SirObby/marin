#include <fstream>
#include <dpp/dpp.h>
#include "nlohmann/json.hpp"

// for convenience
using json = nlohmann::json;

int main()
{
    std::ifstream f("config.json");
    json data = json::parse(f);

    dpp::cluster bot(data["token"]);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([](dpp::slashcommand_t event) -> dpp::job {
	        if (event.command.get_command_name() == "ping") {
	            dpp::cluster *cluster = event.from->creator;

                int pong = ( event.from->websocket_ping + cluster->rest_ping ) * 1000;
                dpp::message m;
                m.set_flags(dpp::m_ephemeral);
                std::string str = "Pinged: " + std::to_string(pong) + "ms";
                m.set_content(str);

                event.reply(dpp::ir_channel_message_with_source, m);
            } 
    });

    bot.on_ready([&bot](const dpp::ready_t &event)
                 {
	        if (dpp::run_once<struct register_bot_commands>()) {
	            dpp::slashcommand ping_command("ping", "Ping pong!", bot.me.id);
                dpp::slashcommand info_command("info", "Info of the bot", bot.me.id);
                dpp::slashcommand profile_command("profile", "Show user profile", bot.me.id);
                profile_command.add_option(dpp::command_option(dpp::co_user, "user", "The user who's profile you want to view", false));
                dpp::slashcommand search_command("search", "Search for an anime from myanimelist", bot.me.id);
                search_command.add_option(dpp::command_option(dpp::co_string, "name", "The name of the anime", true));
                search_command.add_option(dpp::command_option(dpp::co_boolean, "filter", "Filter potentially NSFW results.", false));
                /*search_command.add_option(dpp::command_option(dpp::co_string, "filter", "The type of filter to use", false)
                    .add_choice(dpp::command_option_choice("rx (default)", std::string("rx")))
                    .add_choice(dpp::command_option_choice("none", std::string("none")))
                    .add_choice(dpp::command_option_choice("MAL filter", std::string("nsfw")))
                    );*/

	            bot.global_bulk_command_create({ping_command, info_command, profile_command, search_command});
	        }

            bot.set_presence(dpp::presence(dpp::presence_status::ps_idle, dpp::activity_type::at_listening, "UNDER CONSTRUCTION")); });

    bot.start(dpp::st_wait);
}