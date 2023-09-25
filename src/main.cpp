#include <fstream>
#include <dpp/dpp.h>
#include "nlohmann/json.hpp"
#include "commands/commands.h"
#include <fmt/format.h>
#include "anime.h"
// for convenience
using json = nlohmann::json;

std::string urlEncode(std::string str){
    std::string new_str = "";
    char c;
    int ic;
    const char* chars = str.c_str();
    char bufHex[10];
    int len = strlen(chars);

    for(int i=0;i<len;i++){
        c = chars[i];
        ic = c;
        // uncomment this if you want to encode spaces with +
        /*if (c==' ') new_str += '+';   
        else */if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') new_str += c;
        else {
            sprintf(bufHex,"%X",c);
            if(ic < 16) 
                new_str += "%0"; 
            else
                new_str += "%";
            new_str += bufHex;
        }
    }
    return new_str;
 }


int main()
{
    std::ifstream f("config.json");
    json data = json::parse(f);

    dpp::cluster bot(data["token"]);

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
	            //Commands::search_command(event, data);
                dpp::http_request_completion_t result = co_await event.from->creator->co_request(fmt::format("https://api.myanimelist.net/v2/anime?q={}&limit=1&nsfw=true&fields=id,title,main_picture,start_date,end_date,synopsis,mean,rank,popularity,nsfw,broadcast,source,rating,related_anime,recommendations",urlEncode(std::get<std::string>(event.get_parameter("name")))), dpp::m_get, "", "application/json", {
                    {"X-MAL-CLIENT-ID", data["MAL-CLIENT-ID"]}
                });
                dpp::message m;
                m.set_flags(dpp::m_ephemeral);
                if (result.status == 200) {
	                m.add_embed(generate_anime_embed(result.body));
                    //m.set_content(result.body.substr(0, result.body.find('\n', 0)));
                    //printf(result.body.c_str());
                    //printf(result.body.substr(0, result.body.find('\n', 0)));
	            }
	            event.reply(m);
            }
            if (event.command.get_command_name() == "profile") {
	            Commands::profile_command(event);
            } 
            if (event.command.get_command_name() == "character") {
	            //Commands::profile_command(event);
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
                dpp::slashcommand charac_command("character", "Search for a character from anilist.co", bot.me.id);
                charac_command.add_option(dpp::command_option(dpp::co_string, "name", "The name of the character", true));
                charac_command.add_option(dpp::command_option(dpp::co_boolean, "filter", "Filter potentially NSFW results.", false));

                /*search_command.add_option(dpp::command_option(dpp::co_string, "filter", "The type of filter to use", false)
                    .add_choice(dpp::command_option_choice("rx (default)", std::string("rx")))
                    .add_choice(dpp::command_option_choice("none", std::string("none")))
                    .add_choice(dpp::command_option_choice("MAL filter", std::string("nsfw")))
                    );*/

	            bot.global_bulk_command_create({ping_command, info_command, profile_command, search_command, charac_command});
	        }

            bot.set_presence(dpp::presence(dpp::presence_status::ps_idle, dpp::activity_type::at_listening, "UNDER CONSTRUCTION")); });

    bot.start(dpp::st_wait);
}