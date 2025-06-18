#pragma once
#include "Database.h"
#include "Panel.h"
#include <string>
#include <utility>
#include <vector>

struct Employee {
        int id = -1;
        std::string name = "";
        std::string position = "";
        double salary = 0.00f;
};

class EmployeesPanel : public Panel {
    public:
        EmployeesPanel(Database &db);
        void render() override;
        void refresh();
        bool saveRecord(Employee rec, std::string where);
        Employee getSelEmployee();
        bool isCurrentChanged();

    private:
        Database &db;
        std::vector<Employee> employees;
        int selectedEmployee = -1;
        Employee currentEmployee;
        int old_index = -1; // для отслеживание изменения выделенной строки
};
