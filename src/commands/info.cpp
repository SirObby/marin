#include "commands.h"
#include "../ember.h"
#include <fmt/format.h>
#include "../box/sentry.h"

namespace Commands
{
    void info_command(dpp::slashcommand_t event)
    {
        dpp::cluster *bot = event.from->creator;
        bot->current_application_get([bot, event](const dpp::confirmation_callback_t &v)
                                     {
		dpp::application app;
		uint64_t guild_count = 0;
		if (!v.is_error()) {
			app = std::get<dpp::application>(v.value);
			guild_count = app.approximate_guild_count;
		}
        
        dpp::message m;
        ember emb;
        emb.add_description("I am Marin Kitagawa, a discord bot for searching for information about an anime based on the title.\n\n/search to get started!");
        emb.set_title("Info! ~ ♡");
        emb.add_field("invite the bot", "[here](https://discord.com/api/oauth2/authorize?client_id=1132282262541582347&permissions=313408&scope=bot%20applications.commands)", true);
        emb.add_field("support server", "[here](https://discord.gg/4kH9gWErcP)", true);
        emb.add_field("DPP version", DPP_VERSION_TEXT, true);
        emb.add_field("servers", fmt::format("{}", guild_count), true);
        emb.add_field("sentry version", sentry::version(), true);
        emb.add_field("marin version", "america update!", true);
        
        m.set_content("");
        m.add_embed(emb.return_emb());

        event.reply(dpp::ir_channel_message_with_source, m); });
    };
}
