#pragma once
#include <dpp/dpp.h>

namespace commands {
    dpp::task<void> profile(dpp::slashcommand_t event);
}
