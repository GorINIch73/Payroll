// Settings.h
#pragma once
#include "Database.h"
// #include "Sett /ingsPanel.h"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <string>

class Settings {
    public:
        void Load();
        void Save();
        void AddToHistory(const std::string &filepath);
        void ToggleTheme();
        void getDataFromDB(Database &db);

        // Конфигурационный файл
        const std::string CONFIG_FILE = ".payroll.config.json";

        // Глобальные переменные для хранения истории и темы
        std::vector<std::string> recentFiles;
        ImGuiStyle currentStyle;
        bool darkTheme = true;
        std::string organization = "";
};
