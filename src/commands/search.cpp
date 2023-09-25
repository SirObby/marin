#include "commands.h"
#include <string>
#include <iostream>
#include <fmt/format.h>

namespace Commands
{
    void search_command(dpp::slashcommand_t event, nlohmann::json config) {
        dpp::cluster *cluster = event.from->creator;
        /*
        int pong = ( event.from->websocket_ping + cluster->rest_ping ) * 1000;
        dpp::message m;
        m.set_flags(dpp::m_ephemeral);
        std::string str = "Pinged: " + std::to_string(pong) + "ms";
        m.set_content(str);

        event.reply(dpp::ir_channel_message_with_source, m);*/
        /*cluster->co_request(
	            fmt::format("https://api.myanimelist.net/v2/anime?q={}&nsfw=true&fields=id,title,main_picture,start_date,end_date,synopsis,mean,rank,popularity,nsfw,broadcast,source,rating,related_anime,recommendations",std::get<std::string>(event.get_parameter("name"))), dpp::m_post, [&event](const dpp::http_request_completion_t & cc) {
	                // This callback is called when the HTTP request completes. See documentation of
	                // dpp::http_request_completion_t for information on the fields in the parameter.
	                std::cout << "I got reply: " << cc.body << " with HTTP status code: " << cc.status << "\n";
                    std::cout << fmt::format("https://api.myanimelist.net/v2/anime?q={}&nsfw=true&fields=id,title,main_picture,start_date,end_date,synopsis,mean,rank,popularity,nsfw,broadcast,source,rating,related_anime,recommendations",std::get<std::string>(event.get_parameter("name"))) << std::endl;
	            },
	            "",
	            "application/json",
	            {
	                {"X-MAL-CLIENT-ID", config["MAL-CLIENT-ID"]}
	            }
	        );*/
    };
}
