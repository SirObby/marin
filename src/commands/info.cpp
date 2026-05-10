#include "info.h"
#include <format>

namespace commands {
    void info(dpp::slashcommand_t event) {
        dpp::embed emb = dpp::embed()
            .set_color(0xe7c4ca)
            .set_title("Info! ~ ♡")
            .set_description("I am Marin Kitagawa, a discord bot for searching for information about an anime based on the title.\n\n/search to get started!")
            .add_field("invite the bot", "[here](https://discord.com/api/oauth2/authorize?client_id=1132282262541582347&permissions=313408&scope=bot%20applications.commands)", true)
            .add_field("support server", "[here](https://discord.gg/4kH9gWErcP)", true)
            .add_field("Library", std::format("D++ {}", dpp::utility::version()), true)
            .add_field("Servers", std::to_string(dpp::get_guild_cache()->count()), true);
        event.reply(dpp::message().add_embed(emb));
    }
}
