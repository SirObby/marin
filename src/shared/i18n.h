#pragma once
#include <string>
#include <unordered_map>
#include <dpp/json.h>

class I18n {
private:
    std::unordered_map<std::string, nlohmann::json> locales;

public:
    static I18n& get() {
        static I18n instance;
        return instance;
    }

    void init();
    std::string t(const std::string& key, const std::string& locale = "en", const std::vector<std::string>& args = {});
};
