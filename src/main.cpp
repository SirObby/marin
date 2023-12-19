#include <fstream>
#include <dpp/dpp.h>
#include "nlohmann/json.hpp"
#include "commands/commands.h"
#include <fmt/format.h>
#include "config.h"

#include "anime.h"
// for convenience
using json = nlohmann::json;

#include "box/sentry.h"

int main()
{
    std::ifstream f("config.json");
    json data = json::parse(f);

    dpp::cluster bot(data["token"]);

    sentry::init(bot);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([&data](dpp::slashcommand_t event) -> dpp::job
                        {
	        if (event.command.get_command_name() == "ping") {
	            Commands::ping_command(event);
            } 
            if (event.command.get_command_name() == "info") {
	            Commands::info_command(event);
            } 
            if (event.command.get_command_name() == "search") {
                co_await Commands::search_command(event, data);
            }
            if (event.command.get_command_name() == "profile") {
	            Commands::profile_command(event);
            } 
            if (event.command.get_command_name() == "character") {
                co_await Commands::charac_command(event, data);
            } 
    });

    bot.on_ready([&bot, &data](const dpp::ready_t &event)
                 {
	        if (dpp::run_once<struct register_bot_commands>()) {
	            dpp::slashcommand ping_command("ping", "Ping pong!", bot.me.id);
                dpp::slashcommand info_command("info", "Info of the bot", bot.me.id);
                dpp::slashcommand profile_command("profile", "Show user profile", bot.me.id);
                profile_command.add_option(dpp::command_option(dpp::co_user, "user", "The user who's profile you want to view", false));
                dpp::slashcommand search_command("search", "Search for an anime from myanimelist", bot.me.id);
                search_command.add_option(dpp::command_option(dpp::co_string, "name", "The name of the anime", true));
                search_command.add_option(dpp::command_option(dpp::co_boolean, "filter", "Filter potentially NSFW results.", false));
                dpp::slashcommand charac_command("character", "Search for a character from anilist.co", bot.me.id);
                charac_command.add_option(dpp::command_option(dpp::co_string, "name", "The name of the character", true));
                charac_command.add_option(dpp::command_option(dpp::co_boolean, "filter", "Filter potentially NSFW results.", false));

                /*search_command.add_option(dpp::command_option(dpp::co_string, "filter", "The type of filter to use", false)
                    .add_choice(dpp::command_option_choice("rx (default)", std::string("rx")))
                    .add_choice(dpp::command_option_choice("none", std::string("none")))
                    .add_choice(dpp::command_option_choice("MAL filter", std::string("nsfw")))
                    );*/

	            bot.global_bulk_command_create({ping_command, info_command, profile_command, search_command, charac_command});

                bot.set_presence(dpp::presence(dpp::presence_status::ps_idle, dpp::activity_type::at_custom, "Looking for Gojou-kun."));

                bot.start_timer([&bot, &data](const dpp::timer& timer) {
	                //bot.set_presence(dpp::presence(dpp::presence_status::ps_idle, dpp::activity_type::at_watching, std::to_string(dpp::get_guild_cache()->count()) + " servers"));

                    std::string mypostdata = "{\"server_ccount\":" + std::to_string(dpp::get_guild_cache()->count()) + "\"}";
	        // Make a HTTP POST request. HTTP and HTTPS are supported here.
	        bot.request(
	            "https://top.gg/api/bot/1132282262541582347/stats", dpp::m_post, [&data](const dpp::http_request_completion_t & cc) {
	                // This callback is called when the HTTP request completes. See documentation of
	                // dpp::http_request_completion_t for information on the fields in the parameter.
	                std::cout << "I got reply: " << cc.body << " with HTTP status code: " << cc.status << "\n";
	            },
	            mypostdata,
	            "application/json",
	            {
	                {"Authorization", data["top.gg"]}
	            }
	        );

                }, 120);
	        }
        });

    bot.start(dpp::st_wait);

    sentry::close();
}