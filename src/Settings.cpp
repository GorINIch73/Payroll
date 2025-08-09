#include "Settings.h"
#include "SettingsPanel.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <imgui.h>


using json = nlohmann::json;

void Settings::Load() {

    std::ifstream file(CONFIG_FILE);
    if (file.good()) {
       
        try {
            json config;
            file >> config;
            
            // Загружаем историю файлов
            if (config.contains("recent_files")) {
                recentFiles = config["recent_files"].get<std::vector<std::string>>();
            }
            
            // Загружаем тему
            if (config.contains("dark_theme")) {
                darkTheme = config["dark_theme"];
                if (darkTheme) {
                    ImGui::StyleColorsDark();
                } else {
                    ImGui::StyleColorsLight();
                }
                currentStyle = ImGui::GetStyle();
            }
            
        } catch (const json::exception& e) {
            // Ошибка при чтении файла - используем значения по умолчанию
            recentFiles.clear();
            darkTheme = true;
            ImGui::StyleColorsDark();
            currentStyle = ImGui::GetStyle();
        }
    }
}

void Settings::Save() {

    json config;
    
    // Сохраняем историю файлов (например, последние 10 файлов)
    const size_t maxHistory = 10;
    if (recentFiles.size() > maxHistory) {
        recentFiles.erase(recentFiles.begin(), recentFiles.end() - maxHistory);
    }
    config["recent_files"] = recentFiles;
    
    // Сохраняем тему
    config["dark_theme"] = darkTheme;
    
    std::ofstream file(CONFIG_FILE);
    file << config.dump(4); // Красивое форматирование с отступами
}

// Добавление файла в историю
void Settings::AddToHistory(const std::string& filepath) {


    // Удаляем дубликаты
    auto it = std::find(recentFiles.begin(), recentFiles.end(), filepath);
    if (it != recentFiles.end()) {
        recentFiles.erase(it);
    }
    
    // Добавляем в конец
    recentFiles.push_back(filepath);
    
    // Сохраняем изменения
    Save();

}

// Переключение темы
void Settings::ToggleTheme() {
    darkTheme = !darkTheme;
    if (darkTheme) {
        ImGui::StyleColorsDark();
    } else {
        ImGui::StyleColorsLight();
    }
    currentStyle = ImGui::GetStyle();
    Save();
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

