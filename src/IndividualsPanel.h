#pragma once
#include "Database.h"
#include "Panel.h"
#include <string>
#include <utility>
#include <vector>

struct Individual {
        int id = -1;
        std::string full_name = "";
        std::string note = "";
};

class IndividualsPanel : public Panel {
    public:
        IndividualsPanel(Database &db);
        ~IndividualsPanel();
        void render() override;
        void refresh();
        bool writeToDatabase();
        bool addRecord();
        bool delRecord();
        bool isCurrentChanged();

    private:
        Database &db;
        std::vector<Individual> individuals;
        int selectedIndex = -1;
        Individual currentRecord;
        int oldIndex = -1; // для отслеживание изменения выделенной строки
};
