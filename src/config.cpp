#include "config.h"

#include <iostream>

namespace config {

    json& get(std::string name) {
        
        std::cout << name << std::endl;

        std::ifstream f("config.json");
        json data = json::parse(f);

        std::cout << data.dump(4);

        if (name.empty()) {
			return data;
		}
		return data.at(name);
    };

};