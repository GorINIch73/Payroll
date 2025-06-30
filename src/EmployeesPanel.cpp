
#include "EmployeesPanel.h"
#include "Icons.h"
#include "imgui_stdlib.h"
#include <algorithm>
// #include <cfloat>
// #include <charconv>
#include <cstdio>
#include <cstring>
#include <imgui.h>
#include <iostream>
// #include <ostream>
#include <string>
// #include <unicode/uchar.h> // Для ICU библиотеки
#include <unicode/utf8.h>

#include "imgui_components.h"

EmployeesPanel::EmployeesPanel(Database &db)
    : Panel("Справочник работников"),
      db(db) {
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}

bool EmployeesPanel::writeToDatabase() {
    // Созранение записи из редактора с проверкой изменения
    std::string sql;

    if (isCurrentChanged()) {
        sql =
            "UPDATE Employees SET " +
            (currentRecord.individual_id > 0
                 ? " individual_id=" +
                       std::to_string(currentRecord.individual_id)
                 : "") +
            (currentRecord.position_id > 0
                 ? ", position_id=" + std::to_string(currentRecord.position_id)
                 : "") +
            ", rate=" + std::to_string(currentRecord.rate) + ", contract='" +
            currentRecord.contract + "', contract_found=" +
            std::to_string(currentRecord.contract_found ? 1 : 0) +
            ", certificate_found=" +
            std::to_string(currentRecord.certificate_found ? 1 : 0) +
            ", note='" + currentRecord.note +
            "' WHERE id=" + std::to_string(currentRecord.id) + ";";

        // std::cout << currentRecord.note << std::endl;
        // std::cout << sql << std::endl;
        if (db.execute(sql)) {

            // обновляем запись в таблице пока что полным обновлением
            refresh();
        };
        return false;
    }

    return false;
}

bool EmployeesPanel::addRecord() {
    // добавление новой записи в базу
    std::string sql;
    sql = "INSERT INTO Employees (rate) VALUES (1);";
    return db.execute(sql);
}

bool EmployeesPanel::delRecord() {
    // удаление текущей записи
    if (currentRecord.id >= 0) {
        std::string sql;
        sql = "DELETE FROM Employees WHERE id =" +
              std::to_string(currentRecord.id) + ";";
        return db.execute(sql);
    }

    return false;
}

void EmployeesPanel::refresh() {

    // Основная таблица
    employees.clear();
    std::cout << "рефреш ..." << std::endl;
    // Загружаем данные из БД (упрощенный пример)
    // const char *sql = "SELECT id, individual_id, position_id, rate, contract,
    // "
    //                   "contract_found, certificate_found, note FROM
    //                   Employees;";
    const char *sql = "SELECT e.id, e.individual_id, i.full_name, "
                      "e.position_id, p.job_title, e.rate, e.contract, "
                      "e.contract_found, e.certificate_found, e.note FROM "
                      "Employees e LEFT JOIN Individuals i ON e.individual_id "
                      "= i.id LEFT JOIN Positions p ON e.position_id = p.id;";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<Employee> *>(data);
            // не забываем проверять текстовые поля на NULL

            // std::cout << "argc " << argc << " 1: " << argv[0]
            //           << "0: " << argv[1] << " 1: " << argv[1]
            //           << " 2: " << argv[2] << " 3: " << argv[3] << std::endl;
            list->emplace_back(Employee{
                std::stoi(argv[0]),
                argv[1] ? std::stoi(argv[1]) : -1,
                argv[2] ? argv[2] : "",
                argv[3] ? std::stoi(argv[3]) : -1,
                argv[4] ? argv[4] : "",
                argv[5] ? std::stod(argv[5]) : 0,
                argv[6] ? argv[6] : "",
                argv[7] ? (std::stoi(argv[7]) > 0 ? true : false) : false,
                argv[8] ? (std::stoi(argv[8]) > 0 ? true : false) : false,
                argv[9] ? argv[9] : "",
            });
            return 0;
        },
        &employees, nullptr);

    // std::cout << "рефреш основной " << std::endl;
    // таблица физлиц

    individuals.clear();
    const char *sqlF = "SELECT id, full_name "
                       "FROM Individuals;";
    sqlite3_exec(
        db.getHandle(), sqlF,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<ComboItem> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                ComboItem{std::stoi(argv[0]), argv[1] ? argv[1] : ""});
            return 0;
        },
        &individuals, nullptr);

    // std::cout << "рефреш физлиц норм " << std::endl;
    // таблица должностейш
    positions.clear();
    const char *sqlP = "SELECT id, job_title "
                       "FROM Positions;";
    sqlite3_exec(
        db.getHandle(), sqlP,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<ComboItem> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                ComboItem{std::stoi(argv[0]), argv[1] ? argv[1] : ""});
            return 0;
        },
        &positions, nullptr);

    // std::cout << "рефреш должностей норм" << std::endl;
}

bool EmployeesPanel::isCurrentChanged() {

    // если индексы не определены
    if (currentRecord.id < 0)
        return false;
    if (oldIndex < 0)
        return false;

    // std::cout << " тест " << str << std::endl;
    // std::cout << " новое  :" << currentRecord.note << std::endl;
    // std::cout << " старое :" << employees[oldIndex].note << std::endl;

    // срапвниваем поля
    if (currentRecord.individual_id != employees[oldIndex].individual_id)
        return true;
    if (currentRecord.position_id != employees[oldIndex].position_id)
        return true;
    if (currentRecord.rate != employees[oldIndex].rate)
        return true;
    if (currentRecord.contract != employees[oldIndex].contract)
        return true;
    if (currentRecord.contract_found != employees[oldIndex].contract_found)
        return true;
    if (currentRecord.certificate_found !=
        employees[oldIndex].certificate_found)
        return true;
    if (currentRecord.note != employees[oldIndex].note)
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
        selectedIndex = employees.size() - 1;
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
            oldIndex = -1;
            selectedIndex--;
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

    ImGui::Text("%s %d", "ID :", currentRecord.id);
    // если нужно, то вокус на поле ввода
    if (focusFirst) {
        ImGui::SetKeyboardFocusHere();
        focusFirst = false;
    }

    // ImGui::InputInt("ФИО1", &currentRecord.individual_id);
    // ImGui::InputText("ФИО", &currentRecord.individual);
    ImGui::Text("ФИО");
    ImGui::SameLine();
    // комбобокс с фильтром
    if (ComboWithFilter("##ФИО", &currentRecord.individual_id, individuals)) {
        // Обработка изменения выбора
        auto it3 = std::find_if(individuals.begin(), individuals.end(),
                                [&](const ComboItem &e) {
                                    return e.id == currentRecord.individual_id;
                                });

        if (it3 != individuals.end()) {
            // ImGui::Text("Выбрано: %s (ID: %d)", it3->name.c_str(), it3->id);
            currentRecord.individual_id = it3->id;
        }
    }
    ImGui::Text("Примечание");
    // текст с автопереносом
    InputTextWrapper("##note", currentRecord.note,
                     ImGui::GetContentRegionAvail().x);

    // Таблица со списком

    // ImGui::SameLine();
    // ImGui::SeparatorText("справочник");

    // ImGui::PushStyleColor(ImGuiCol_Separator,
    //                       ImVec4(1, 0, 0, 1)); // Красный цвет
    ImGui::Separator();
    // ImGui::PopStyleColor();

    if (ImGui::BeginTable("Employees", 3,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn("ID",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                50.0f);
        ImGui::TableSetupColumn("ФИО");
        ImGui::TableSetupColumn("Примечание");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < employees.size(); ++i) {

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            // выделение всей    строки
            if (ImGui::Selectable(std::to_string(employees[i].id).c_str(),
                                  selectedIndex == i,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                selectedIndex = i;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", employees[i].individual.c_str());
            ImGui::TableSetColumnIndex(2);
            // обрабатываем многострочку - просто срезаем после возврата строки
            ImGui::Text("%s", employees[i]
                                  .note.substr(0, employees[i].note.find("\n"))
                                  .c_str());
            // ImGui::TableSetColumnIndex(3);
            // // выравнивание вправо
            // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            // ImGui::SetCursorPosX(
            //     ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
            //     ImGui::CalcTextSize(
            //         std::to_string(employees[i].salary).c_str())
            //         .x -
            //     ImGui::GetStyle().ItemSpacing.x);
            // ImGui::Text("%.2f", employees[i].salary);
            // ImGui::PopStyleVar();
            //
            // выделена другая строка
            if (oldIndex != selectedIndex) {
                // записать старые данные
                if (writeToDatabase()) {
                    // Обновляем строку таблицы новыми данными
                    employees[oldIndex] = currentRecord;
                    // printf("Запись обновлена\n");
                }
                // printf("старый указатель %i, новый указатель %i \n",
                // oldIndex,
                //        selectedEmployee);
                // взять в редактор новые данные
                currentRecord = employees[selectedIndex];
                oldIndex = selectedIndex;
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
