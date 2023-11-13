#if !defined(_H_COMMANDS)
#define _H_COMMANDS

#include <dpp/dpp.h>
#include "../nlohmann/json.hpp"

namespace Commands
{
    void ping_command(dpp::slashcommand_t event);
    void info_command(dpp::slashcommand_t event);
    void profile_command(dpp::slashcommand_t event);
    auto search_command(dpp::slashcommand_t event, nlohmann::json config) -> dpp::task<void>;
    auto charac_command(dpp::slashcommand_t event, nlohmann::json config) -> dpp::task<void>;
}


#endif // _H_COMMANDS
