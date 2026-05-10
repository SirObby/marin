#include "../src/shared/i18n.h"
#include <cassert>
#include <iostream>

void test_i18n() {
    std::cout << "Testing I18n...\n";
    I18n::get().init(); // Requires locales/ folder
    
    std::string en_text = I18n::get().t("INFO_DESC", "en");
    assert(!en_text.empty() && en_text != "INFO_DESC");
    
    std::string es_text = I18n::get().t("INFO_DESC", "es");
    assert(!es_text.empty() && es_text != "INFO_DESC");
    
    // Fallback test
    std::string fallback = I18n::get().t("INFO_DESC", "fr");
    assert(fallback == en_text);
    
    // Formatting test
    std::string formatted = I18n::get().t("PING_PONG", "en", {"15"});
    assert(formatted == "Pong! Latency: 15ms");
}
