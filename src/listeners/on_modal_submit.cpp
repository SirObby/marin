#include "listeners.h"
#include "../shared/database.h"
#include "../shared/i18n.h"

namespace listeners {
    dpp::task<void> on_form_submit(const dpp::form_submit_t& event) {
        std::string custom_id = event.custom_id;
        std::string locale = Database::get().get_user_locale(event.command.usr.id);
        
        if (custom_id.starts_with("modal_rate_")) {
            std::string type = custom_id.substr(11, 5); // ANIME or MANGA
            uint64_t id = std::stoull(custom_id.substr(17));
            
            int rating = 0;
            std::string rating_str;
            if (!event.components.empty() && !event.components[0].components.empty()) {
                rating_str = std::get<std::string>(event.components[0].components[0].value);
            }
            if (!rating_str.empty()) {
                try {
                    rating = std::stoi(rating_str);
                } catch(...) {}
            }

            bool success = Database::get().add_favorite(event.command.usr.id, id, type, rating);
            if (success) {
                event.reply(dpp::message(I18n::get().t("FAVORITE_ADDED", locale)).set_flags(dpp::m_ephemeral));
            } else {
                event.reply(dpp::message(I18n::get().t("FAVORITE_FAILED", locale)).set_flags(dpp::m_ephemeral));
            }
        }
        co_return;
    }
}
