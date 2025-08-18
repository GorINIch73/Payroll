#pragma once
#include "Database.h"
#include "Panel.h"
#include <string>
#include <utility>
#include <vector>

struct SettingDB {
        int id = -1;
        std::string organization = "";
        std::string note = "";
};

class SettingsPanel : public Panel {
    public:
        SettingsPanel(Database &db);
        ~SettingsPanel();
        void render() override;
        void refresh();
        bool writeToDatabase();

    private:
        Database &db;
        std::vector<SettingDB> settingsDB;
};
