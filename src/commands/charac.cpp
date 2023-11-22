#include "commands.h"
#include <string>
#include <iostream>
#include <fmt/format.h>

namespace Commands
{
    auto charac_command(dpp::slashcommand_t event, nlohmann::json config) -> dpp::task<void> {
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

        dpp::message m;
        //m.set_flags(dpp::m_ephemeral);
        //m.set_content("I am searching.");
        
        dpp::async thinking = event.co_thinking(false);

        json variables;
        variables["search"] = std::get<std::string>(event.get_parameter("name"));
        json pdata;
        pdata["variables"] = variables;
        pdata["query"] = "query ($search: String) {\n  Character(search: $search) {\n    name {\n      first\n      middle\n      last\n      full\n      native\n      userPreferred\n    }\n    image {\n      large\n      medium\n    }\n    age\n    description\n    gender\n  }\n}";
                
        std::string postdata = pdata.dump();
	    std::cout << postdata;
        dpp::http_request_completion_t http_req = co_await cluster->co_request(
            "https://graphql.anilist.co/",
            dpp::m_post,
            postdata,
                "application/json",
	            {
	                //{"Authorization", "Bearer tokengoeshere"}
                }
        );

        std::cout << http_req.body << std::endl;

        m.set_content("Here's what I got from AniList.co:");

        dpp::embed emb = dpp::embed().set_color(std::stol("e7c4ca", nullptr, 16));

        json query_data = json::parse(http_req.body);

        if(query_data["data"]["Character"].is_null()) {
            co_await thinking;

            event.edit_response(dpp::message().add_embed(dpp::embed().set_color(std::stol("ff0000", nullptr, 16)).set_description("Looks like a search result was not found (or the API is down)").set_title("Error")));
            co_return;
        }


        if(query_data["data"]["Character"]["name"]["native"].is_string() ) 
            emb.set_title(query_data["data"]["Character"]["name"]["native"].template get<std::string>());

        emb.set_description(fmt::format("{}\n", (query_data["data"]["Character"]["name"]["full"].is_string()) ? query_data["data"]["Character"]["name"]["native"].template get<std::string>()
                : ""
        ));
        
        if(query_data["data"]["Character"]["id"].is_number())
            emb.set_url(std::format("https://anilist.co/character/{}", query_data["data"]["Character"]["id"].template get<int>()));
        
        if(query_data["data"]["Character"]["gender"].is_string())
            emb.add_field("Gender", query_data["data"]["Character"]["gender"].template get<std::string>(), true);

        if(query_data["data"]["Character"]["age"].is_string())
            emb.add_field("Age", query_data["data"]["Character"]["age"].template get<std::string>(), true);

        if(query_data["data"]["Character"]["image"]["large"].is_string())
            emb.set_thumbnail(query_data["data"]["Character"]["image"]["large"].template get<std::string>());

        //emb.set_thumbnail("");

        co_await thinking;
        event.edit_response(m);

    };
}
