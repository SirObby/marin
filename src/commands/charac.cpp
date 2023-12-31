#include "commands.h"
#include <string>
#include <iostream>
#include <fmt/format.h>

using json = nlohmann::json;

namespace Commands
{
    auto charac_command(dpp::slashcommand_t event, nlohmann::json config) -> dpp::task<void> {
        dpp::cluster *cluster = event.from->creator;

        dpp::message m;
        
        dpp::async thinking = event.co_thinking(false);

        json variables;
        variables["search"] = std::get<std::string>(event.get_parameter("name"));
        json pdata;
        pdata["variables"] = variables;
        pdata["query"] = "query ($search: String) {\n  Character(search: $search) {\n    name {\n      first\n      middle\n      last\n      full\n      native\n      userPreferred\n    }\n    image {\n      large\n      medium\n    }\n    age\n    description\n    gender\n    id\n  }\n}";
                
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

        emb.set_description(fmt::format("{}\n", (query_data["data"]["Character"]["name"]["full"].is_string()) ? query_data["data"]["Character"]["name"]["full"].template get<std::string>()
                : ""
        ));
        
        if(query_data["data"]["Character"]["id"].is_number())
            emb.set_url(fmt::format("https://anilist.co/character/{}", query_data["data"]["Character"]["id"].template get<int>()));
        
        if(query_data["data"]["Character"]["gender"].is_string())
            emb.add_field("Gender", query_data["data"]["Character"]["gender"].template get<std::string>(), true);

        if(query_data["data"]["Character"]["age"].is_string())
            emb.add_field("Age", query_data["data"]["Character"]["age"].template get<std::string>(), true);

        if(query_data["data"]["Character"]["image"]["large"].is_string())
            emb.set_thumbnail(query_data["data"]["Character"]["image"]["large"].template get<std::string>());

        //emb.set_thumbnail("");

        m.add_embed(emb);

        co_await thinking;
        event.edit_response(m);

    };
}
