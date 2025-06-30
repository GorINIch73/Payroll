#pragma once
#include "Database.h"
#include "Panel.h"
#include "imgui_components.h"
#include <string>
#include <utility>
#include <vector>

struct Employee {
        int id = -1;
        int individual_id = -1;
        std::string individual = "";
        int position_id = -1;
        std::string position = "";
        double rate;
        std::string contract = "";
        bool contract_found = false;
        bool certificate_found = false;
        std::string note = "";
};

// struct ListItem {
//         int id = -1;
//         std::string value = "";
// };
//
class EmployeesPanel : public Panel {
    public:
        EmployeesPanel(Database &db);
        void render() override;
        void refresh();
        bool writeToDatabase();
        bool addRecord();
        bool delRecord();
        bool isCurrentChanged();

    private:
        Database &db;
        std::vector<Employee> employees;    // основная таблица
        std::vector<ComboItem> individuals; // таблица физлиц для комбо
        std::vector<ComboItem> positions;   // таблица должностей для комбо

        int selectedIndex = -1;
        Employee currentRecord;
        int oldIndex = -1; // для отслеживание изменения выделенной строки
};
