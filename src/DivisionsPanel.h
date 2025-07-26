#pragma once
#include "Database.h"
#include "Panel.h"
#include <string>
#include <utility>
#include <vector>

struct Division {
        int id = -1;
        std::string division_name = "";
        std::string note = "";
};

class DivisionsPanel : public Panel {
    public:
        DivisionsPanel(Database &db);
        ~DivisionsPanel();
        void render() override;
        void refresh();
        bool writeToDatabase();
        bool addRecord();
        bool delRecord();
        bool isCurrentChanged();

    private:
        Database &db;
        std::vector<Division> divisions;
        int selectedIndex = -1;
        Division currentRecord;
        int oldIndex = -1; // для отслеживание изменения выделенной строки
};
