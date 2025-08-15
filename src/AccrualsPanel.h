#pragma once
#include "Database.h"
#include "Panel.h"
#include <string>
#include <utility>
#include <vector>

struct Accrual {
        int id = -1;
        std::string name = "";
        double percentage = 0;
        bool this_salary = false;
        bool verification = false;
        std::string note = "";
};

class AccrualsPanel : public Panel {
    public:
        AccrualsPanel(Database &db);
        ~AccrualsPanel();
        void render() override;
        void refresh();
        bool writeToDatabase();
        bool addRecord();
        bool delRecord();
        bool isCurrentChanged();

    private:
        Database &db;
        std::vector<Accrual> accruals;
        int selectedIndex = -1;
        Accrual currentRecord;
        int oldIndex = -1; // для отслеживание изменения выделенной строки
};
