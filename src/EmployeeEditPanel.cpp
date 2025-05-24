
#include "EmployeeEditPanel.h"
#include <imgui.h>
#include <cstring>

EmployeeEditPanel::EmployeeEditPanel(Database& db, EmployeeListPanel& listPanel) 
    : Panel("Редактирование сотрудника"), db(db), listPanel(listPanel) {
    memset(nameBuffer, 0, sizeof(nameBuffer));
    memset(positionBuffer, 0, sizeof(positionBuffer));
}

void EmployeeEditPanel::setEmployee(const Employee& emp) {
    currentEmployee = emp;
    strncpy(nameBuffer, emp.name.c_str(), sizeof(nameBuffer));
    strncpy(positionBuffer, emp.position.c_str(), sizeof(positionBuffer));
    salary = emp.salary;
}

void EmployeeEditPanel::render() {
    if (!isOpen) return;
    
    setEmployee(listPanel.GetSelEmployee());

//    ImGui::Begin(name.c_str(), &isOpen);

    ImGui::BeginChild(name.c_str());


    ImGui::InputText("Имя", nameBuffer, sizeof(nameBuffer));
    ImGui::InputText("Должность", positionBuffer, sizeof(positionBuffer));
    ImGui::InputDouble("Зарплата", &salary, 0.0, 0.0, "%.2f");

    if (ImGui::Button("Сохранить")) {
        std::string sql;
        if (currentEmployee.id == 0) {  // Новый сотрудник
            sql = "INSERT INTO Employees (name, position, salary) VALUES ('" +
                  std::string(nameBuffer) + "', '" + positionBuffer + "', " + std::to_string(salary) + ");";
        } else {  // Обновление
            sql = "UPDATE Employees SET name='" + std::string(nameBuffer) + 
                  "', position='" + positionBuffer + 
                  "', salary=" + std::to_string(salary) + 
                  " WHERE id=" + std::to_string(currentEmployee.id) + ";";
        }
        if (db.execute(sql)) {
            listPanel.refresh();  // Обновляем список
        }
    }

//    ImGui::End();

    ImGui::EndChild();
}
