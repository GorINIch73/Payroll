// Settings.h
#pragma once
#include "Database.h"
// #include "Sett /ingsPanel.h"
#include <imgui.h>
#include <string>
#include <vector>

class Settings {
    public:
        void Load();
        void Save();
        void AddToHistory(const std::string &filepath);
        void ToggleTheme();
        void SetTheme(int theme);
        const char *GetThemeName(int theme) const;
        int GetThemeCount() const;
        void getDataFromDB(Database &db);
        void LoadFonts(float size = 0.0f);

        // Конфигурационный файл
        const std::string CONFIG_FILE = ".payroll.config";

        // Глобальные переменные для хранения истории и темы
        std::vector<std::string> recentFiles;
        ImGuiStyle currentStyle;
        bool darkTheme = true;
        int theme = 0;
        float sizeFont = 24.0f;
        std::string organization = "";
        bool needLoadFonts = true;

    private:
        void ApplyTheme();
};

inline Settings settings;
