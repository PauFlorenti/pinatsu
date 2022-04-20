#pragma once

#include <external/json/json.hpp>
using json = nlohmann::json;

json loadJson(const std::string& filename);