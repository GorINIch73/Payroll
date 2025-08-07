#include "Settings.h"
#include "SettingsPanel.h"
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

void Settings::getDataFromDB(Database &db) {

    // Загружаем данные из БД нужна 1 строка 
    // дублирование с панелью, но хз как сделать

    std::vector<SettingDB> settingsDB;
    const char *sql = "SELECT id, organization, note FROM Settings WHERE id = ? LIMIT 1;";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<SettingDB> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(SettingDB{std::stoi(argv[0]), argv[1],
                                          argv[2] ? argv[2] : ""});
            return 0;
        },
        &settingsDB, nullptr);
    // пока только наименование организации
    organization=settingsDB[0].organization;
    
}
