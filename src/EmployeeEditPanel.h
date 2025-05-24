#pragma once
#include "Panel.h"
#include "Database.h"
#include "EmployeeListPanel.h"

class EmployeeEditPanel : public Panel {
public:
    EmployeeEditPanel(Database& db, EmployeeListPanel& listPanel);
    void render() override;
    void setEmployee(const Employee& emp);

private:
    Database& db;
    EmployeeListPanel& listPanel;
    Employee currentEmployee;
    char nameBuffer[256];
    char positionBuffer[256];
    double salary = 0.0;
};
