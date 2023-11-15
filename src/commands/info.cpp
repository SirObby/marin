#include "commands.h"
#include "../ember.h"

namespace Commands
{
    void info_command(dpp::slashcommand_t event) {
//        dpp::cluster *cluster = event.from->creator;

//                int pong = ( event.from->websocket_ping + cluster->rest_ping ) * 1000;
                dpp::message m;
//                m.set_flags(dpp::m_ephemeral);
//                std::string str = "Pinged: " + std::to_string(pong) + "ms";
                ember emb;
                emb.add_description("I am Marin Kitagawa, a discord bot for receiving anime recommendations based on your anime tastes.\n\n/get-started to get started!");
                emb.set_title("Info! ~ ♡");
                emb.add_field("invite the bot", "[here](https://discord.com/api/oauth2/authorize?client_id=1132282262541582347&permissions=313408&scope=bot%20applications.commands)", true);
                emb.add_field("support server", "[here](https://discord.gg/4kH9gWErcP)", true);
                m.set_content("");
                m.add_embed(emb.return_emb());

                event.reply(dpp::ir_channel_message_with_source, m);
    };
}
