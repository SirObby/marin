#pragma once
#include <dpp/dpp.h>

namespace commands {
    dpp::task<void> character(dpp::slashcommand_t event);
}
