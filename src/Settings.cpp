#include "Settings.h"
#include <fstream>
#include <nlohmann/json.hpp>

void Settings::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    nlohmann::json json;
    file >> json;
    
    if (json.contains("lastDbPath")) lastDbPath = json["lastDbPath"];
    if (json.contains("theme")) theme = json["theme"];
}

void Settings::Save(const std::string& path) {
    nlohmann::json json;
    json["lastDbPath"] = lastDbPath;
    json["theme"] = theme;

    std::ofstream file(path);
    file << json.dump(4);
}