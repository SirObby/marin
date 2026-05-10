#include "ping.h"
#include "../shared/i18n.h"
#include "../shared/database.h"

namespace commands {
    void ping(dpp::slashcommand_t event) {
        dpp::cluster* bot = event.from()->creator;
        std::string locale = Database::get().get_user_locale(event.command.usr.id);
        
        auto shard = bot->get_shard(event.from()->shard_id);
        int ping_ms = 0;
        if (shard) {
            ping_ms = static_cast<int>(shard->websocket_ping * 1000.0);
        }
        
        std::string text = I18n::get().t("PING_PONG", locale, {std::to_string(ping_ms)});
        event.reply(dpp::message(text));
    }
}
