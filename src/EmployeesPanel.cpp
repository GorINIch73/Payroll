
#include "EmployeesPanel.h"
#include "imgui_stdlib.h"
#include <imgui.h>
#include <string>

EmployeesPanel::EmployeesPanel(Database &db)
    : Panel("Справочник сотрудников"),
      db(db) {
    refresh();
}

Employee EmployeesPanel::getSelEmployee() {

    Employee cur = Employee{0, "", "", 0.0};

    if (selectedEmployee < 0)
        return cur;

    cur.id = employees[selectedEmployee].id;
    cur.name = employees[selectedEmployee].name;
    cur.position = employees[selectedEmployee].position;
    cur.salary = employees[selectedEmployee].salary;

    return cur;
}

void EmployeesPanel::refresh() {
    employees.clear();
    // Загружаем данные из БД (упрощенный пример)
    const char *sql = "SELECT id, name, position, salary FROM Employees;";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<Employee> *>(data);
            list->emplace_back(Employee{std::stoi(argv[0]), argv[1], argv[2],
                                        std::stod(argv[3])});
            return 0;
        },
        &employees, nullptr);
}

bool EmployeesPanel::isCurrentChanged() {

    // если индексы не опрадалены
    if (currentEmployee.id < 0)
        return false;
    if (old_index < 0)
        return false;

    // срапвниваем поля
    if (currentEmployee.name != employees[old_index].name)
        return true;
    if (currentEmployee.position != employees[old_index].position)
        return true;
    if (currentEmployee.salary != employees[old_index].salary)
        return true;

    return false;
}

void EmployeesPanel::render() {
    if (!isOpen)
        return;

    //    ImGui::Begin(name.c_str(), &isOpen);

    ImGui::BeginChild(name.c_str());

    // Кнопка "Обновить"
    // ImGui::BeginChild("123");
    if (ImGui::Button("Обновить")) {
        refresh();
    }
    // ImGui::EndChild();
    // поля редактирования
    // ImGui::SeparatorText("редактор");

    ImGui::Text("%s %d", "ID :", currentEmployee.id);
    ImGui::InputText("Имя", &currentEmployee.name);
    // ImGui::InputText("Должность", positionBuffer, sizeof(positionBuffer));
    ImGui::InputText("Должность", &currentEmployee.position);
    ImGui::InputDouble("Зарплата", &currentEmployee.salary, 0.0, 0.0, "%.2f");

    if (ImGui::Button("Сохранить")) {
        std::string sql;
        if (currentEmployee.id < 0) { // Новый сотрудник
            //     sql = "INSERT INTO Employees (name, position, salary) VALUES
            //     ('" +
            //           std::string(nameBuffer) + "', '" + positionBuffer + "',
            //           " + std::to_string(salary) + ");";
            sql = "INSERT INTO Employees (name, position, salary) VALUES ('" +
                  currentEmployee.name + "', '" + currentEmployee.position +
                  "', " + std::to_string(currentEmployee.salary) + ");";
        } else { // Обновление
            if (isCurrentChanged())
                sql = "UPDATE Employees SET name='" + currentEmployee.name +
                      "', position='" + currentEmployee.position +
                      "', salary=" + std::to_string(currentEmployee.salary) +
                      " WHERE id=" + std::to_string(currentEmployee.id) + ";";
        }
        if (db.execute(sql)) {
            refresh(); // Обновляем список
        }
    }

    // Таблица со списком
    // ImGui::SameLine();
    // ImGui::SeparatorText("справочник");

    // ImGui::PushStyleColor(ImGuiCol_Separator,
    //                       ImVec4(1, 0, 0, 1)); // Красный цвет
    ImGui::Separator();
    // ImGui::PopStyleColor();

    if (ImGui::BeginTable("Employees", 4,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Имя");
        ImGui::TableSetupColumn("Должность");
        ImGui::TableSetupColumn("Зарплата");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < employees.size(); ++i) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            // выделение всей    строки
            if (ImGui::Selectable(std::to_string(employees[i].id).c_str(),
                                  selectedEmployee == i,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                selectedEmployee = i;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", employees[i].name.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", employees[i].position.c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.2f", employees[i].salary);
            if (old_index != selectedEmployee) {
                // выделена другай строка
                if (isCurrentChanged()) {
                    std::string sql;

                    sql =
                        "UPDATE Employees SET name='" + currentEmployee.name +
                        "', position='" + currentEmployee.position +
                        "', salary=" + std::to_string(currentEmployee.salary) +
                        " WHERE id=" + std::to_string(currentEmployee.id) + ";";

                    if (db.execute(sql)) {
                        // Обновляем строку таблицы новыми данными
                        employees[old_index] = currentEmployee;
                        printf("Запись обновлена\n");
                    }
                }
                // из таблицы в поля редактирования
                currentEmployee = employees[i];
                old_index = selectedEmployee;
            }
        }
        ImGui::EndTable();
    }

    //    ImGui::End();

    // ImGui::EndChild();

    // ImGui::BeginChild();
    // setEmployee(listPanel.GetSelEmployee());

    //    ImGui::Begin(name.c_str(), &isOpen);

    // ImGui::BeginChild(name.c_str());

    //    ImGui::End();

    ImGui::EndChild();
}
