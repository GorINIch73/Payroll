#pragma once
#include "Database.h"
#include "Panel.h"
#include <string>
#include <utility>
#include <vector>

struct Position {
        int id = -1;
        std::string job_title = "";
        double salary = 0.0f;
        double norm = 0.0f;
        std::string note = "";
};

class PositionsPanel : public Panel {
    public:
        PositionsPanel(Database &db);
        ~PositionsPanel();

        void render() override;
        void refresh();
        bool writeToDatabase();
        bool addRecord();
        bool delRecord();
        bool isCurrentChanged();

    private:
        Database &db;
        std::vector<Position> positions;
        int selectedIndex = -1;
        Position currentRecord;
        int oldIndex = -1; // для отслеживание изменения выделенной строки
};
