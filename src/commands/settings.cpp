#include "settings.h"
#include "../shared/database.h"
#include "../shared/i18n.h"
#include <string>

namespace commands {
    void settings(dpp::slashcommand_t event) {
        dpp::command_interaction cmd_data = event.command.get_command_interaction();
        std::string locale = Database::get().get_user_locale(event.command.usr.id);

        if (cmd_data.options.empty()) {
            event.reply(dpp::message("Please specify a subcommand.").set_flags(dpp::m_ephemeral));
            return;
        }

        if (cmd_data.options[0].name == "user") {
            auto& user_opts = cmd_data.options[0].options;
            bool updated = false;
            for (auto& opt : user_opts) {
                if (opt.name == "locale") {
                    std::string lang = std::get<std::string>(opt.value);
                    Database::get().set_user_locale(event.command.usr.id, lang);
                    locale = lang;
                    updated = true;
                } else if (opt.name == "bio") {
                    std::string bio = std::get<std::string>(opt.value);
                    Database::get().set_user_bio(event.command.usr.id, bio);
                    updated = true;
                }
            }
            if (updated) event.reply(dpp::message(I18n::get().t("SETTINGS_UPDATED", locale)).set_flags(dpp::m_ephemeral));
            else event.reply(dpp::message("No settings changed.").set_flags(dpp::m_ephemeral));
        } else if (cmd_data.options[0].name == "guild") {
            if (event.command.guild_id == 0) {
                event.reply(dpp::message("Guild settings can only be used in a server.").set_flags(dpp::m_ephemeral));
                return;
            }
            auto& guild_opts = cmd_data.options[0].options;
            bool updated = false;
            for (auto& opt : guild_opts) {
                if (opt.name == "nsfw") {
                    bool allow = std::get<bool>(opt.value);
                    Database::get().set_guild_nsfw(event.command.guild_id, allow);
                    updated = true;
                } else if (opt.name == "theme") {
                    std::string hex = std::get<std::string>(opt.value);
                    if (hex[0] == '#') hex = hex.substr(1);
                    try {
                        uint32_t color = std::stoul(hex, nullptr, 16);
                        Database::get().set_guild_theme(event.command.guild_id, color);
                        updated = true;
                    } catch (...) {}
                }
            }
            if (updated) event.reply(dpp::message(I18n::get().t("SETTINGS_GUILD_UPDATED", locale)).set_flags(dpp::m_ephemeral));
            else event.reply(dpp::message("No settings changed.").set_flags(dpp::m_ephemeral));
        }
    }
}
