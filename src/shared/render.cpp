#include "render.h"
#include <format>
#include <string>

namespace render {

    // Helper to safely get string from json
    std::string get_string(const nlohmann::json& node, const std::string& key, const std::string& def) {
        if (node.contains(key) && node[key].is_string()) {
            return node[key].get<std::string>();
        }
        return def;
    }

    // Helper to safely get nested string
    std::string get_nested_string(const nlohmann::json& node, const std::string& key1, const std::string& key2, const std::string& def) {
        if (node.contains(key1) && node[key1].is_object() && node[key1].contains(key2) && node[key1][key2].is_string()) {
            return node[key1][key2].get<std::string>();
        }
        return def;
    }

    dpp::embed media(const nlohmann::json& node) {
        dpp::embed emb = dpp::embed()
            .set_color(THEME_COLOR)
            .set_footer("Marin ♡", "https://images-ext-2.discordapp.net/external/yuWQyYHNamIH8o5RXB9CWATQPtT54N_UZBfcJA5-AXk/https/i.pinimg.com/originals/a2/25/42/a22542523b0ecb54f1abdc6df3373336.gif");

        if (node.contains("title") && node["title"].is_object()) {
            std::string title = get_string(node["title"], "romaji");
            if (title.empty()) title = get_string(node["title"], "english");
            if (title.empty()) title = get_string(node["title"], "native");
            if (!title.empty()) emb.set_title(title);
        }

        if (node.contains("id") && node["id"].is_number()) {
            std::string type = get_string(node, "type", "anime");
            // lowercase type
            for(auto& c : type) c = std::tolower(c);
            emb.set_url(std::format("https://anilist.co/{}/{}", type, node["id"].get<int>()));
        }

        std::string desc = get_string(node, "description");
        if (!desc.empty()) {
            // Strip some basic html tags from description
            size_t pos = 0;
            while((pos = desc.find("<br>")) != std::string::npos) desc.replace(pos, 4, "\n");
            while((pos = desc.find("<i>")) != std::string::npos) desc.replace(pos, 3, "*");
            while((pos = desc.find("</i>")) != std::string::npos) desc.replace(pos, 4, "*");
            while((pos = desc.find("<b>")) != std::string::npos) desc.replace(pos, 3, "**");
            while((pos = desc.find("</b>")) != std::string::npos) desc.replace(pos, 4, "**");
            
            if (desc.length() > 2000) {
                desc = desc.substr(0, 2000) + "...";
            }
            emb.set_description(desc);
        }

        std::string image = get_nested_string(node, "coverImage", "extraLarge");
        if (image.empty()) image = get_nested_string(node, "coverImage", "large");
        if (!image.empty()) emb.set_thumbnail(image);

        if (node.contains("genres") && node["genres"].is_array() && !node["genres"].empty()) {
            std::string genres;
            for (const auto& genre : node["genres"]) {
                if (genre.is_string()) {
                    if (!genres.empty()) genres += ", ";
                    genres += genre.get<std::string>();
                }
            }
            emb.add_field("Genres", genres, true);
        }

        if (node.contains("episodes") && !node["episodes"].is_null()) {
            emb.add_field("Episodes", std::to_string(node["episodes"].get<int>()), true);
        }

        if (node.contains("nextAiringEpisode") && !node["nextAiringEpisode"].is_null()) {
            auto nextAir = node["nextAiringEpisode"];
            int ep = nextAir["episode"].get<int>();
            int time = nextAir["airingAt"].get<int>();
            emb.add_field("Next Episode (" + std::to_string(ep) + ")", std::format("<t:{}:R>", time), true);
        }

        if (node.contains("chapters") && !node["chapters"].is_null()) {
            emb.add_field("Chapters", std::to_string(node["chapters"].get<int>()), true);
        }

        if (node.contains("averageScore") && node["averageScore"].is_number()) {
            emb.add_field("Score", std::to_string(node["averageScore"].get<int>()) + "/100", true);
        }
        
        if (node.contains("status") && node["status"].is_string()) {
            emb.add_field("Status", node["status"].get<std::string>(), true);
        }

        if (node.contains("isAdult") && node["isAdult"].is_boolean() && node["isAdult"].get<bool>()) {
            emb.set_color(0xff0000); // Red for mature
        }

        return emb;
    }

    dpp::embed character(const nlohmann::json& node) {
        dpp::embed emb = dpp::embed()
            .set_color(THEME_COLOR)
            .set_footer("Marin ♡", "https://images-ext-2.discordapp.net/external/yuWQyYHNamIH8o5RXB9CWATQPtT54N_UZBfcJA5-AXk/https/i.pinimg.com/originals/a2/25/42/a22542523b0ecb54f1abdc6df3373336.gif");

        if (node.contains("name") && node["name"].is_object()) {
            std::string name = get_string(node["name"], "full");
            if (name.empty()) name = get_string(node["name"], "native");
            if (!name.empty()) emb.set_title(name);
        }

        if (node.contains("id") && node["id"].is_number()) {
            emb.set_url(std::format("https://anilist.co/character/{}", node["id"].get<int>()));
        }

        std::string desc = get_string(node, "description");
        if (!desc.empty()) {
            size_t pos = 0;
            while((pos = desc.find("<br>")) != std::string::npos) desc.replace(pos, 4, "\n");
            while((pos = desc.find("__")) != std::string::npos) desc.replace(pos, 2, "**");
            if (desc.length() > 2000) {
                desc = desc.substr(0, 2000) + "...";
            }
            emb.set_description(desc);
        }

        std::string image = get_nested_string(node, "image", "large");
        if (!image.empty()) emb.set_thumbnail(image);

        if (node.contains("gender") && node["gender"].is_string()) {
            emb.add_field("Gender", node["gender"].get<std::string>(), true);
        }
        
        if (node.contains("age") && node["age"].is_string()) {
            emb.add_field("Age", node["age"].get<std::string>(), true);
        }

        return emb;
    }

    dpp::embed staff(const nlohmann::json& node) {
        dpp::embed emb = dpp::embed()
            .set_color(THEME_COLOR)
            .set_footer("Marin ♡", "https://images-ext-2.discordapp.net/external/yuWQyYHNamIH8o5RXB9CWATQPtT54N_UZBfcJA5-AXk/https/i.pinimg.com/originals/a2/25/42/a22542523b0ecb54f1abdc6df3373336.gif");

        if (node.contains("name") && node["name"].is_object()) {
            std::string name = get_string(node["name"], "full");
            if (name.empty()) name = get_string(node["name"], "native");
            if (!name.empty()) emb.set_title(name);
        }

        if (node.contains("id") && node["id"].is_number()) {
            emb.set_url(std::format("https://anilist.co/staff/{}", node["id"].get<int>()));
        }

        std::string desc = get_string(node, "description");
        if (!desc.empty()) {
            if (desc.length() > 2000) {
                desc = desc.substr(0, 2000) + "...";
            }
            emb.set_description(desc);
        }

        std::string image = get_nested_string(node, "image", "large");
        if (!image.empty()) emb.set_thumbnail(image);

        if (node.contains("primaryOccupations") && node["primaryOccupations"].is_array() && !node["primaryOccupations"].empty()) {
            std::string occs;
            for (const auto& occ : node["primaryOccupations"]) {
                if (occ.is_string()) {
                    if (!occs.empty()) occs += ", ";
                    occs += occ.get<std::string>();
                }
            }
            emb.add_field("Occupations", occs, true);
        }

        return emb;
    }

    dpp::embed studio(const nlohmann::json& node) {
        dpp::embed emb = dpp::embed()
            .set_color(THEME_COLOR)
            .set_footer("Marin ♡", "https://images-ext-2.discordapp.net/external/yuWQyYHNamIH8o5RXB9CWATQPtT54N_UZBfcJA5-AXk/https/i.pinimg.com/originals/a2/25/42/a22542523b0ecb54f1abdc6df3373336.gif");

        std::string name = get_string(node, "name");
        if (!name.empty()) emb.set_title(name);

        if (node.contains("id") && node["id"].is_number()) {
            emb.set_url(std::format("https://anilist.co/studio/{}", node["id"].get<int>()));
        }

        if (node.contains("isAnimationStudio") && node["isAnimationStudio"].is_boolean()) {
            emb.add_field("Type", node["isAnimationStudio"].get<bool>() ? "Animation Studio" : "Studio", true);
        }

        return emb;
    }

}
