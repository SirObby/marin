#pragma once
#include <dpp/dpp.h>

namespace commands {
    dpp::task<void> search(dpp::slashcommand_t event);
}
