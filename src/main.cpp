#include <fstream>
#include <iostream>
#include <dpp/dpp.h>
#include <dpp/json.h>

#include "commands/ping.h"
#include "commands/info.h"
#include "commands/profile.h"
#include "commands/search.h"
#include "commands/character.h"
#include "commands/manga.h"
#include "commands/settings.h"
#include "commands/delete_data.h"

#include "listeners/listeners.h"

#include "shared/database.h"
#include "shared/i18n.h"

using json = nlohmann::json;

int main()
{
    std::ifstream f("config.json");
    if (!f.is_open()) {
        std::cerr << "Failed to open config.json\n";
        return 1;
    }
    json data = json::parse(f);

    Database::get().init("user=postgres dbname=marin sslmode=disable");
    I18n::get().init();

    dpp::cluster bot(data["token"].get<std::string>());
    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([](const dpp::slashcommand_t& event) -> dpp::task<void> {
        std::string cmd = event.command.get_command_name();
        if (cmd == "ping") commands::ping(event);
        else if (cmd == "info") commands::info(event);
        else if (cmd == "profile") co_await commands::profile(event);
        else if (cmd == "search") co_await commands::search(event);
        else if (cmd == "character") co_await commands::character(event);
        else if (cmd == "manga") co_await commands::manga(event);
        else if (cmd == "settings") commands::settings(event);
        else if (cmd == "delete_data") commands::delete_data(event);
        co_return;
    });

    bot.on_select_click(listeners::on_select_click);
    bot.on_button_click(listeners::on_button_click);
    bot.on_form_submit(listeners::on_form_submit);

    bot.on_guild_delete([](const dpp::guild_delete_t& event) {
        Database::get().delete_guild(event.deleted.id);
    });

    bot.on_ready([&bot, &data](const dpp::ready_t &event) {
        (void)event;
        if (dpp::run_once<struct register_bot_commands>()) {
            std::vector<dpp::slashcommand> commands;

            commands.push_back(dpp::slashcommand("ping", "Ping pong!", bot.me.id));
            commands.push_back(dpp::slashcommand("info", "Info of the bot", bot.me.id));
            
            dpp::slashcommand profile_command("profile", "Show user profile favorites", bot.me.id);
            profile_command.add_option(dpp::command_option(dpp::co_user, "user", "The user who's profile you want to view", false));
            commands.push_back(profile_command);
            
            dpp::slashcommand search_command("search", "Search for an anime", bot.me.id);
            search_command.add_option(dpp::command_option(dpp::co_string, "name", "The name of the anime", false));
            search_command.add_option(dpp::command_option(dpp::co_integer, "id", "The Anilist ID of the anime", false));
            commands.push_back(search_command);
            
            dpp::slashcommand charac_command("character", "Search for a character", bot.me.id);
            charac_command.add_option(dpp::command_option(dpp::co_string, "name", "The name of the character", true));
            commands.push_back(charac_command);

            dpp::slashcommand manga_command("manga", "Search for a manga", bot.me.id);
            manga_command.add_option(dpp::command_option(dpp::co_string, "name", "The name of the manga", false));
            manga_command.add_option(dpp::command_option(dpp::co_integer, "id", "The Anilist ID of the manga", false));
            commands.push_back(manga_command);

            dpp::slashcommand settings_command("settings", "Bot settings", bot.me.id);
            
            dpp::command_option user_opt(dpp::co_sub_command, "user", "User Settings");
            dpp::command_option locale_opt(dpp::co_string, "locale", "Language", false);
            locale_opt.add_choice(dpp::command_option_choice("English", std::string("en")));
            locale_opt.add_choice(dpp::command_option_choice("Español", std::string("es")));
            user_opt.add_option(locale_opt);
            user_opt.add_option(dpp::command_option(dpp::co_string, "bio", "Your profile biography", false));
            
            dpp::command_option guild_opt(dpp::co_sub_command, "guild", "Guild Settings (Admin)");
            guild_opt.add_option(dpp::command_option(dpp::co_boolean, "nsfw", "Allow mature content", false));
            guild_opt.add_option(dpp::command_option(dpp::co_string, "theme", "Theme color hex (e.g. #ff0000)", false));
            
            settings_command.add_option(user_opt);
            settings_command.add_option(guild_opt);
            commands.push_back(settings_command);

            commands.push_back(dpp::slashcommand("delete_data", "Delete all your data from the bot", bot.me.id));

            bot.global_bulk_command_create(commands);
            bot.set_presence(dpp::presence(dpp::presence_status::ps_idle, dpp::activity_type::at_custom, "Looking for Gojou-kun."));

            bot.start_timer([&bot, &data](const dpp::timer& timer){
                (void)timer;
                std::string mypostdata = "{\"server_count\": " + std::to_string(dpp::get_guild_cache()->count()) + "}";
                // Make a HTTP POST request. HTTP and HTTPS are supported here.
                bot.request(
                    "https://top.gg/api/v1/projects/@me/metrics", dpp::m_patch, [](const dpp::http_request_completion_t & cc) {
                        // This callback is called when the HTTP request completes. See documentation of
                        // dpp::http_request_completion_t for information on the fields in the parameter.
                        std::cout << "I got reply: " << cc.body << " with HTTP status code: " << cc.status << "\n";
                    },
                    mypostdata,
                    "application/json",
                    {
                        {"Authorization", std::format("Bearer {}", data["topgg"].get<std::string>())}
                    }
                );
            }, 3600); // Do it every hour.
        }
    });

    bot.start(dpp::st_wait);
    return 0;
}
