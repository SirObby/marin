#include "anime.h"
#include <fmt/format.h>
#include "ember.h"

dpp::embed generate_anime_embed(std::string return_body) {
    nlohmann::json j = ::json::parse(return_body);
    
    ember emb;

    if(j["data"].is_null()) return emb.add_description("no data").return_emb();
    nlohmann::json data = j["data"];
    if(data[0].is_null()) return emb.add_description("no data[0]").return_emb();
    nlohmann::json node = data[0];

    printf("%s %d", data[0].dump().c_str(), node["id"].template get<int>());

    std::string content_body = fmt::format("");
    if(!node["id"].is_null()) emb.set_url(fmt::format("https://myanimelist.net/anime/{}/", node["id"].template get<int>()));
    if(!node["title"].is_null()) emb.set_title(fmt::format("{}", node["title"].template get<std::string>()));
    if(!node["main_picture"]["medium"].is_null()) emb.set_thumbnail(fmt::format("{}", node["main_picture"]["medium"].template get<std::string>()));
    if(!node["synopsis"].is_null()) emb.set_thumbnail(fmt::format("{}", node["synopsis"].template get<std::string>().substr(0, node["synopsis"].template get<std::string>().find('\n', 0))));
    return emb.return_emb();
};