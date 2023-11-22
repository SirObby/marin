#include "commands.h"
#include <string>
#include <iostream>
#include <fmt/format.h>

namespace Commands
{
    auto search_command(dpp::slashcommand_t event, nlohmann::json config) -> dpp::task<void>  {
        dpp::cluster *cluster = event.from->creator;

        dpp::async thinking = event.co_thinking(false);

        json variables;
        variables["search"] = std::get<std::string>(event.get_parameter("name"));
        json pdata;
        pdata["variables"] = variables;
        pdata["query"] = "query ($search: String) {\n  Media (search: $search, type: ANIME) { # Insert our variables into the query arguments (id) (type: ANIME is hard-coded in the query)\n		title {\n      romaji\n      english\n      native\n      userPreferred\n    }\n    format\n    description\n    episodes\n    duration\n    source\n    coverImage {\n      extraLarge\n      large\n      medium\n      color\n    }\n    bannerImage\n    averageScore\n    isAdult\n    genres\n    tags {\n      name\n      rank\n      isMediaSpoiler\n    }\n    id\n    nextAiringEpisode {\n      episode\n      airingAt\n      timeUntilAiring\n    }\n    airingSchedule {\n      edges {\n        id\n        node {\n          airingAt\n          timeUntilAiring\n        }\n      }\n    }\n  }\n}\n";

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

        dpp::message m;

        m.set_content("anilist reponse: ");

        dpp::embed emb = dpp::embed().set_color(std::stol("e7c4ca", nullptr, 16));

        json query_data = json::parse(http_req.body);

        if(query_data["data"]["Media"].is_null()) {
            co_await thinking;

            event.edit_response(dpp::message().add_embed(dpp::embed().set_color(std::stol("ff0000", nullptr, 16)).set_description("Looks like a search result was not found (or the API is down)").set_title("Error")));
            co_return;
        }

        //if(query_data["data"]["title"]["romaji"].is_string()) emb.set_title(query_data["data"]["title"]["romaji"].template get<std::string>());
        if(query_data["data"]["Media"]["title"]["native"].is_string()) { emb.set_title(fmt::format("{}", query_data["data"]["Media"]["title"]["native"].template get<std::string>())); 
            printf("title");
        }
        if(query_data["data"]["Media"]["coverImage"]["large"].is_string() ) { emb.set_thumbnail(query_data["data"]["Media"]["coverImage"]["large"].template get<std::string>());
            printf("bannerimg");
        } 
        if(query_data["data"]["Media"]["description"].is_string()) { emb.set_description(query_data["data"]["Media"]["description"].template get<std::string>());
            printf("desc");
        }
        if(query_data["data"]["Media"]["id"].is_number()) { emb.set_url(fmt::format("https://anilist.co/anime/{}", query_data["data"]["Media"]["id"].template get<int>()));
            printf("desc");
        }
        if(!query_data["data"]["Media"]["genres"].is_null()) {
            std::string genres = "";

            for (auto it = query_data["data"]["Media"]["genres"].begin(); it != query_data["data"]["Media"]["genres"].end(); ++it)
            {
                std::cout << *it << std::endl;
                genres.append(*it);
                genres.append(" ");
            }
            
            emb.add_field("Genres", genres, true);
        }

        if(query_data["data"]["Media"]["episodes"].is_number() && query_data["data"]["Media"]["duration"].is_number()) emb.add_field("Episodes", fmt::format("{}, {}min", query_data["data"]["Media"]["episodes"].template get<int>(), query_data["data"]["Media"]["duration"].template get<int>()), true);
        if(query_data["data"]["Media"]["source"].is_string() ) emb.add_field("Source", query_data["data"]["Media"]["source"].template get<std::string>(), true);
        //emb.set_thumbnail("");
        if(!(query_data["data"]["Media"]["nextAiringEpisode"].is_null())) emb.add_field("Airing", fmt::format("Episode {} <t:{}:f>",query_data["data"]["Media"]["nextAiringEpisode"]["episode"].template get<int>(),query_data["data"]["Media"]["nextAiringEpisode"]["airingAt"].template get<int>()), true);
        

        m.add_embed(emb);        

        if(query_data["data"]["Media"]["isAdult"].is_boolean()) if(query_data["data"]["Media"]["isAdult"].template get<bool>() == true) m.add_embed(dpp::embed().set_color(std::stol("ffff00", nullptr, 16)).set_description("The above is meant for mature audiences.").set_title("Mature Content."));

        co_await thinking;
        event.edit_response(m);

    };
}
