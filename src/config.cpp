#include "config.h"


namespace config {

    json& get(std::string name) {
        std::ifstream f("config.json");
        json data = json::parse(f);

        if (name.empty()) {
			return data;
		}
		return data.at(name);
    };

};