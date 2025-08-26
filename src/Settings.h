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
        void LoadFonts(float size = 0.0f);

        // Конфигурационный файл
        const std::string CONFIG_FILE = ".payroll.config.json";

        // Глобальные переменные для хранения истории и темы
        std::vector<std::string> recentFiles;
        ImGuiStyle currentStyle;
        bool darkTheme = true;
        float sizeFont = 24.0f;
        std::string organization = "";
        bool needLoadFonts = true;
};

inline Settings settings;
