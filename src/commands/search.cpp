#include "commands.h"
#include <string>
#include <iostream>
#include <fmt/format.h>

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

namespace Commands
{
    auto search_command(dpp::slashcommand_t event, nlohmann::json config) -> dpp::task<void>  {
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

        dpp::async thinking = event.co_thinking(false);

        dpp::http_request_completion_t mal_http_req = co_await cluster->co_request(
            fmt::format("https://api.myanimelist.net/v2/anime?q={}&nsfw=true&fields=id,title&limit=1", urlEncode(std::get<std::string>(event.get_parameter("name")))),
            dpp::m_get,
            "",
                "application/json",
	            {
	                {"X-MAL-CLIENT-ID", config["MAL-CLIENT-ID"]}
                }
        );

        std::cout << mal_http_req.body << std::endl;

        json mal_http_req_data = json::parse(mal_http_req.body);
        json datanode = mal_http_req_data["data"];
        if(datanode[0]["node"]["id"].is_null()) {
            co_await thinking;

            event.edit_response(dpp::message().add_embed(dpp::embed().set_color(std::stol("ff0000", nullptr, 16)).set_description("Looks like a search result was not found (or the API is down)").set_title("Error")));
            co_return;
        }
        int id = datanode[0]["node"]["id"].template get<int>();

        json variables;
        variables["search"] = std::to_string(id);
        json pdata;
        pdata["variables"] = variables;
        //pdata["query"] = "query ($search: String) {\n  Character(search: $search) {\n    name {\n      first\n      middle\n      last\n      full\n      native\n      userPreferred\n    }\n    image {\n      large\n      medium\n    }\n    age\n    description\n    gender\n  }\n}";
        pdata["query"] = "query ($search: Int) {\n  Media (idMal: $search, type: ANIME) { # Insert our variables into the query arguments (id) (type: ANIME is hard-coded in the query)\n		title {\n      romaji\n      english\n      native\n      userPreferred\n    }\n    format\n    description\n    episodes\n    duration\n    source\n    coverImage {\n      extraLarge\n      large\n      medium\n      color\n    }\n    bannerImage\n    averageScore\n    isAdult\n    genres\n    tags {\n      name\n      rank\n      isMediaSpoiler\n    }\n    id\n    airingSchedule {\n      edges {\n        id\n      }\n    }\n  }\n}";

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

        //if(query_data["data"]["title"]["romaji"].is_string()) emb.set_title(query_data["data"]["title"]["romaji"].template get<std::string>());
        if(query_data["data"]["Media"]["title"]["romaji"].is_string() && query_data["data"]["Media"]["title"]["native"].is_string()) { emb.set_title(fmt::format("{} {}", query_data["data"]["Media"]["title"]["romaji"].template get<std::string>(), query_data["data"]["Media"]["title"]["native"].template get<std::string>())); 
            printf("title");
        }
        if(query_data["data"]["Media"]["coverImage"]["medium"].is_string()) { emb.set_thumbnail(query_data["data"]["Media"]["coverImage"]["medium"].template get<std::string>());
            printf("bannerimg");
        } 
        if(query_data["data"]["Media"]["description"].is_string()) { emb.set_description(query_data["data"]["Media"]["description"].template get<std::string>());
            printf("desc");
        }
        if(query_data["data"]["Media"]["id"].is_number()) { emb.set_url(fmt::format("https://anilist.co/anime/{}", query_data["data"]["Media"]["id"].template get<int>()));
            printf("desc");
        }
        if(query_data["data"]["Media"]["genres"].is_array()) {
            std::string genres = "";

            for (size_t i = 0; i < query_data["data"]["Media"]["genres"].array().size(); i++)
            {
                genres.append(query_data["data"]["Media"]["genres"].array()[i].dump());
            }
            
            emb.add_field("Genres", genres, true);
        }

        //emb.set_thumbnail("");

        m.add_embed(emb);        

        co_await thinking;
        event.edit_response(m);

    };
}
