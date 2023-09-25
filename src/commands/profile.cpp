#include "commands.h"

namespace Commands
{
    void profile_command(dpp::slashcommand_t event) {
        /*dpp::cluster *cluster = event.from->creator;

                int pong = ( event.from->websocket_ping + cluster->rest_ping ) * 1000;
                dpp::message m;
                m.set_flags(dpp::m_ephemeral);
                std::string str = "Pinged: " + std::to_string(pong) + "ms";
                m.set_content(str);

                event.reply(dpp::ir_channel_message_with_source, m);*/
    };
}
