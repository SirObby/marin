#include "i18n.h"
#include <fstream>
#include <iostream>

void I18n::init() {
    std::vector<std::string> langs = {"en", "es"};
    for (const auto& lang : langs) {
        std::ifstream f("locales/" + lang + ".json");
        if (f.is_open()) {
            try {
                locales[lang] = nlohmann::json::parse(f);
                std::cout << "Loaded locale: " << lang << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse locale " << lang << ": " << e.what() << std::endl;
            }
        } else {
            std::cerr << "Could not open locale file: locales/" << lang << ".json" << std::endl;
        }
    }
}

std::string I18n::t(const std::string& key, const std::string& locale, const std::vector<std::string>& args) {
    std::string lang = locale;
    if (locales.find(lang) == locales.end()) {
        lang = "en"; // fallback
    }

    std::string text;
    if (locales[lang].contains(key) && locales[lang][key].is_string()) {
        text = locales[lang][key].get<std::string>();
    } else if (locales["en"].contains(key) && locales["en"][key].is_string()) {
        text = locales["en"][key].get<std::string>();
    } else {
        return key; // return key if missing
    }

    // Simple formatting {0}, {1}
    for (size_t i = 0; i < args.size(); ++i) {
        std::string placeholder = "{" + std::to_string(i) + "}";
        size_t pos = 0;
        while ((pos = text.find(placeholder, pos)) != std::string::npos) {
            text.replace(pos, placeholder.length(), args[i]);
            pos += args[i].length();
        }
    }

    return text;
}
