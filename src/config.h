#if !defined(_H_CONFIG)
#define _H_CONFIG

#include "nlohmann/json.hpp"
#include <fstream>

using json = nlohmann::json;

namespace config {

    json& get(std::string name);

};

#endif // _H_CONFIG
