#pragma once
#include <dpp/dpp.h>

namespace listeners {
    dpp::task<void> on_select_click(const dpp::select_click_t& event);
    dpp::task<void> on_button_click(const dpp::button_click_t& event);
    dpp::task<void> on_form_submit(const dpp::form_submit_t& event);
}
