#pragma once
#include "Panel.h"
#include "Database.h"
#include <vector>
#include <utility>

struct Employee {
    int id;
    std::string name;
    std::string position;
    double salary;
};

class EmployeeListPanel : public Panel {
public:
    EmployeeListPanel(Database& db);
    void render() override;
    void refresh();
    Employee GetSelEmployee();

private:
    Database& db;
    std::vector<Employee> employees;
    int selectedEmployee = -1;
};
