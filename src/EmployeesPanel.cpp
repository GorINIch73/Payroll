
#include "EmployeesPanel.h"
#include "Icons.h"
#include "imgui_stdlib.h"
#include <cstdio>
#include <imgui.h>
#include <string>

EmployeesPanel::EmployeesPanel(Database &db)
    : Panel("Справочник сотрудников"),
      db(db) {
    refresh();
}

bool EmployeesPanel::writeToDatabase() {
    // Созранение записи из редактора с проверкой изменения
    std::string sql;
    if (isCurrentChanged()) {
        sql = "UPDATE Employees SET name='" + currentEmployee.name +
              "', position='" + currentEmployee.position +
              "', salary=" + std::to_string(currentEmployee.salary) +
              " WHERE id=" + std::to_string(currentEmployee.id) + ";";
        return db.execute(sql);
    }

    return false;
}

bool EmployeesPanel::addRecord() {
    // добавление новой записи в базу
    std::string sql;
    sql = "INSERT INTO Employees (name, position, salary) VALUES ( '', '', "
          "0.00 );";
    return db.execute(sql);
}

bool EmployeesPanel::delRecord() {
    // удаление текущей записи
    if (currentEmployee.id >= 0) {
        std::string sql;
        sql = "DELETE FROM Employees WHERE id =" +
              std::to_string(currentEmployee.id) + ";";
        return db.execute(sql);
    }

    return false;
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
    // проверка на существование таблицы - вдруг база пауста или не та
    if (!db.tableExists("Employees")) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1),
                           "Табилца сотрудников отсуствует!");
        return;
    }

    //    ImGui::Begin(name.c_str(), &isOpen);
    bool goBottom = false;
    bool focusFirst = false;

    ImGui::BeginChild(name.c_str());

    ImGui::BeginGroup();
    // ImGui::Begin("Toolbar");
    // {

    // обновить
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.2f, 0.7f, 0.9f, 1.0f)); // голубой
    if (ImGui::Button(ICON_FA_REFRESH)) {
        refresh();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Обновить данные");
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    // добавление записи
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.2f, 0.7f, 0.2f, 1.0f)); // Зеленый
    if (ImGui::Button(ICON_FA_PLUS)) {
        addRecord();
        refresh();
        // прыгвем на последнюю запись
        selectedEmployee = employees.size() - 1;
        goBottom = true;
        focusFirst = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Добавить нового сотрудника");
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    // удаление
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.9f, 0.1f, 0.1f, 1.0f)); // красный
    if (ImGui::Button(ICON_FA_TRASH)) {                    /* ... */

        ImGui::OpenPopup("Удаление");
    }

    ImGui::PopStyleColor();
    // обработка удаления
    if (ImGui::BeginPopupModal("Удаление", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Внимание!");
        ImGui::Text("Удаление выбранной записи.");
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Продолжить", ImVec2(120, 40))) {

            // Выполнняем удаление
            delRecord();
            ImGui::CloseCurrentPopup();
            refresh();
            // дергаем индекс, что бы система перечитала выделенное
            old_index = -1;
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button("Отмена", ImVec2(120, 40))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Внимание!");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1, 0, 0, 1),
                           "Удаление выбранного сотрудника!");
        // ImGui::BulletText("Удаление выбранного сотрудника");
        ImGui::EndTooltip();
    }

    // }
    // ImGui::End();

    ImGui::EndGroup();
    // ImGui::EndChild();
    // поля редактирования
    // ImGui::SeparatorText("редактор");

    ImGui::Text("%s %d", "ID :", currentEmployee.id);
    // если нужно, то вокус на поле ввода
    if (focusFirst) {
        ImGui::SetKeyboardFocusHere();
        focusFirst = false;
    }
    ImGui::InputText("Имя", &currentEmployee.name);
    ImGui::InputText("Должность", &currentEmployee.position);
    ImGui::InputDouble("Зарплата", &currentEmployee.salary, 0.0, 0.0, "%.2f");

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
            // выравнивание вправо
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
                ImGui::CalcTextSize(std::to_string(employees[i].salary).c_str())
                    .x -
                ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%.2f", employees[i].salary);
            ImGui::PopStyleVar();

            // выделена другая строка
            if (old_index != selectedEmployee) {
                // записать старые данные
                if (writeToDatabase()) {
                    // Обновляем строку таблицы новыми данными
                    employees[old_index] = currentEmployee;
                    printf("Запись обновлена\n");
                }
                // printf("старый указатель %i, новый указатель %i \n",
                // old_index,
                //        selectedEmployee);
                // взять в редактор новые данные
                currentEmployee = employees[selectedEmployee];
                old_index = selectedEmployee;
            }
            // Прокручиваем к последнему элементу если выделена последняя строка
            // - для добавленной записи
            if (goBottom && i == employees.size() - 1) {
                ImGui::SetScrollHereY(1.0f); // 1.0f = нижний край экрана
                goBottom = false;
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
