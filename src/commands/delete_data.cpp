#include "delete_data.h"
#include "../shared/database.h"
#include "../shared/i18n.h"

namespace commands {
    void delete_data(dpp::slashcommand_t event) {
        std::string locale = Database::get().get_user_locale(event.command.usr.id);
        Database::get().delete_user_data(event.command.usr.id);
        event.reply(dpp::message(I18n::get().t("DATA_DELETED", locale)).set_flags(dpp::m_ephemeral));
    }
}
