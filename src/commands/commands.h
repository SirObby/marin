#if !defined(_H_COMMANDS)
#define _H_COMMANDS

#include <dpp/dpp.h>
#include "../nlohmann/json.hpp"

namespace Commands
{
    void ping_command(dpp::slashcommand_t event);
    void info_command(dpp::slashcommand_t event);
    void profile_command(dpp::slashcommand_t event);
    void search_command(dpp::slashcommand_t event, nlohmann::json config);
    auto charac_command(dpp::slashcommand_t event, nlohmann::json config) -> dpp::job ;
}


#endif // _H_COMMANDS
