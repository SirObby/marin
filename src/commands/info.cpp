#include "info.h"
#include <format>

namespace commands {
    void info(dpp::slashcommand_t event) {
        dpp::embed emb = dpp::embed()
            .set_color(0xe7c4ca)
            .set_title("Marin Bot")
            .set_description("A Discord bot for interfacing with the AniList API.")
            .add_field("Library", std::format("D++ {}", dpp::utility::version()), true);
        event.reply(dpp::message().add_embed(emb));
    }
}
