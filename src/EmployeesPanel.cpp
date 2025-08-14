
#include "EmployeesPanel.h"
#include "DivisionsPanel.h"
#include "Icons.h"
#include "IndividualsPanel.h"
#include "Manager.h"
#include "Panel.h"
#include "PositionsPanel.h"
#include "ReviewPanel.h"
#include "imgui_stdlib.h"
#include <algorithm>
// #include <cfloat>
// #include <charconv>
#include <cstdio>
#include <cstring>
#include <imgui.h>
#include <iostream>
// #include <ostream>
#include <memory>
#include <string>
// #include <unicode/uchar.h> // Для ICU библиотеки
#include <unicode/utf8.h>
#include <utility>

#include "imgui_components.h"

EmployeesPanel::EmployeesPanel(Database &db)
    : Panel("Справочник работников"),
      db(db) {
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}

EmployeesPanel::~EmployeesPanel() { writeToDatabase(); }

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
            ", rate=" + std::to_string(currentRecord.rate) +
            (currentRecord.division_id > 0
                 ? ", division_id=" + std::to_string(currentRecord.division_id)
                 : "") +
            ", contract='" + currentRecord.contract + "', contract_found=" +
            std::to_string(currentRecord.contract_found ? 1 : 0) +
            ", certificate_found=" +
            std::to_string(currentRecord.certificate_found ? 1 : 0) +
            ", note='" + currentRecord.note +
            "' WHERE id=" + std::to_string(currentRecord.id) + ";";

        // std::cout << currentRecord.note << std::endl;
        // std::cout << sql << std::endl;
        if (db.Execute(sql)) {

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
    return db.Execute(sql);
}

bool EmployeesPanel::delRecord() {
    // удаление текущей записи
    if (currentRecord.id >= 0) {
        std::string sql;
        sql = "DELETE FROM Employees WHERE id =" +
              std::to_string(currentRecord.id) + ";";
        return db.Execute(sql);
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
    const char *sql =
        "SELECT e.id, e.individual_id, i.full_name, e.position_id, p.job_title,"
        "printf('%.2f', salary) , e.rate, e.division_id, d.division_name, "
        "e.contract,"
        "e.contract_found, e.certificate_found, e.note FROM  Employees e "
        "LEFT JOIN Individuals i ON e.individual_id= i.id LEFT JOIN Positions "
        "p "
        "ON e.position_id = p.id LEFT JOIN Divisions d ON e.division_id = d.id";
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
                argv[6] ? std::stod(argv[6]) : 0,
                argv[7] ? std::stoi(argv[7]) : -1,
                argv[8] ? argv[8] : "",
                argv[9] ? argv[9] : "",
                argv[10] ? (std::stoi(argv[10]) > 0 ? true : false) : false,
                argv[11] ? (std::stoi(argv[11]) > 0 ? true : false) : false,
                argv[12] ? argv[12] : "",
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
    const char *sqlP =
        "SELECT id, job_title || ' : ' || printf('%.2f', salary) "
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

    // таблица отделений
    divisions.clear();
    const char *sqlD = "SELECT id, division_name "
                       "FROM Divisions;";
    sqlite3_exec(
        db.getHandle(), sqlD,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<ComboItem> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                ComboItem{std::stoi(argv[0]), argv[1] ? argv[1] : ""});
            return 0;
        },
        &divisions, nullptr);

    // std::cout << "рефреш отделений норм" << std::endl;
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
    if (currentRecord.division_id != employees[oldIndex].division_id)
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
        writeToDatabase();
        refresh();
        // дергаем индекс, что бы система перечитала выделенное
        oldIndex = -1;
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
        if (selectedIndex >= 0)
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

    // Глобальный фильтр
    ImGui::SameLine();
    ImGui::Text("Фильтр:");
    ImGui::SameLine();
    global_filter.Draw("##global_filter",
                       ImGui::GetContentRegionAvail().x - 100);
    // global_filter.Draw("##global_filter",
    // ImGui::GetContentRegionAvail().x*0.8f);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Фильтр");
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ERASER)) {
        global_filter.Clear();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Очистить фильтр");
    }

    // Отчет по запросу
    ImGui::SameLine();
    // добавление записи
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.5f, 1.0f)); //
    if (ImGui::Button(ICON_FA_LIST)) {

        // Добавляем пнель запроса
        auto newPanel = std::make_unique<ReviewPanel>(
            db,
            "SELECT e.id, i.full_name, p.job_title, e.rate, e.contract,"
            "e.contract_found, e.certificate_found, e.note FROM  Employees e "
            "LEFT JOIN Individuals i ON e.individual_id = i.id LEFT JOIN "
            "Positions p ON e.position_id = p.id");
        // auto newPanel = std::make_unique<PositionsPanel>(db);
        manager_panels.addPanel(std::move(newPanel));
        manager_panels.getNextEnd() = true;
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Отчет по запросу");
    }
    ImGui::PopStyleColor();

    // ImGui::End();

    ImGui::EndGroup();
    // ImGui::EndChild();
    // поля редактирования
    // ImGui::SeparatorText("редактор");

    ImGui::Separator();

    ImGui::Text("%s %d", "ID :", currentRecord.id);
    // если нужно, то вокус на поле ввода
    if (focusFirst) {
        ImGui::SetKeyboardFocusHere();
        focusFirst = false;
    }

    // ImGui::InputInt("ФИО1", &currentRecord.individual_id);
    // ImGui::InputText("ФИО", &currentRecord.individual);
    ImGui::Text("ФИО:");
    ImGui::SameLine();
    // открыть панель физлиц
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.2f, 0.5f, 0.2f, 1.0f)); // Зеленый
    if (ImGui::Button(ICON_FA_USER)) {
        auto newPanel = std::make_unique<IndividualsPanel>(db);
        manager_panels.addPanel(std::move(newPanel));
        manager_panels.getNextEnd() = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Физлица");
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    // комбобокс с фильтром
    if (ComboWithFilter("##ФИО", currentRecord.individual_id, individuals)) {
        // Обработка изменения выбора
        auto it = std::find_if(individuals.begin(), individuals.end(),
                               [&](const ComboItem &e) {
                                   return e.id == currentRecord.individual_id;
                               });

        if (it != individuals.end()) {
            // ImGui::Text("Выбрано: %s (ID: %d)", it3->name.c_str(),
            currentRecord.individual_id = it->id;
        }
    }

    ImGui::Text("Должность:");
    ImGui::SameLine();
    // открыть панель должностей
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.2f, 0.5f, 0.2f, 1.0f)); // Зеленый
    if (ImGui::Button(ICON_FA_TIE)) {
        auto newPanel = std::make_unique<PositionsPanel>(db);
        manager_panels.addPanel(std::move(newPanel));
        manager_panels.getNextEnd() = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Должности");
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    // комбобокс с фильтром
    if (ComboWithFilter("##Должность", currentRecord.position_id, positions)) {
        // Обработка изменения выбора
        auto itp = std::find_if(positions.begin(), positions.end(),
                                [&](const ComboItem &e) {
                                    return e.id == currentRecord.position_id;
                                });

        if (itp != positions.end()) {
            currentRecord.position_id = itp->id;
        }
    }

    ImGui::Text("Занято ставок:");
    ImGui::SameLine();
    ImGui::SetCursorPosX(
        ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
        ImGui::CalcTextSize(std::to_string(currentRecord.rate).c_str()).x);
    ImGui::SetNextItemWidth(
        ImGui::CalcTextSize(std::to_string(currentRecord.rate).c_str()).x);
    ImGui::InputDouble("##ставки", &currentRecord.rate, 0, 0, "%0.2f");

    ImGui::Text("Отделение:");
    ImGui::SameLine();
    // открыть панель отделений
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.2f, 0.5f, 0.2f, 1.0f)); // Зеленый
    if (ImGui::Button(ICON_FA_DIVISIONS)) {
        auto newPanel = std::make_unique<DivisionsPanel>(db);
        manager_panels.addPanel(std::move(newPanel));
        manager_panels.getNextEnd() = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Отделения");
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    // комбобокс с фильтром
    if (ComboWithFilter("##DIV", currentRecord.division_id, divisions)) {
        // Обработка изменения выбора
        auto it = std::find_if(divisions.begin(), divisions.end(),
                               [&](const ComboItem &e) {
                                   return e.id == currentRecord.division_id;
                               });

        if (it != divisions.end()) {
            // ImGui::Text("Выбрано: %s (ID: %d)", it3->name.c_str(),
            currentRecord.division_id = it->id;
        }
    }

    ImGui::Text("Трудовой договор:");
    ImGui::SameLine();
    ImGui::InputText("##договор", &currentRecord.contract);

    ToggleButton("Договор найден:", currentRecord.contract_found);

    ImGui::SameLine();
    ToggleButton("Сертификат найден:", currentRecord.certificate_found);

    ImGui::Text("Примечание:");
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

    if (ImGui::BeginTable("Employees", 10,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn("ID",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                50.0f);
        ImGui::TableSetupColumn("ФИО");
        ImGui::TableSetupColumn("Должность");
        ImGui::TableSetupColumn("Оклад",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                150.0f);
        ImGui::TableSetupColumn("Ставка",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                100.0f);
        ImGui::TableSetupColumn("Отделение");
        ImGui::TableSetupColumn("Договор");
        ImGui::TableSetupColumn("дог+",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                10.0f);
        ImGui::TableSetupColumn("серт+",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                10.0f);
        ImGui::TableSetupColumn("Примечание");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < employees.size(); ++i) {

            // фильр - определяем нудна ли нам текущая строка для отображения

            // Собираем всю строку в один текст для фильтрации
            std::string row_text;
            row_text += std::to_string(employees[i].id) + " ";
            row_text += employees[i].individual + " ";
            row_text += employees[i].position + " ";
            row_text += std::to_string(employees[i].salary) + " ";
            row_text += std::to_string(employees[i].rate) + " ";
            row_text += employees[i].division + " ";
            row_text += employees[i].contract + " ";
            row_text += employees[i].note;
            // если не совпадает с фильтром, то пропускаем строку
            if (!global_filter.PassFilter(row_text.c_str())) {
                continue;
            }

            // таблица
            ImGui::TableNextRow();
            // ID
            ImGui::TableSetColumnIndex(0);
            // выделение всей    строки
            if (ImGui::Selectable(std::to_string(employees[i].id).c_str(),
                                  selectedIndex == i,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                selectedIndex = i;
            }

            // ФИО
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", employees[i].individual.c_str());
            // Должность
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", employees[i].position.c_str());
            // оклад
            ImGui::TableSetColumnIndex(3);
            // прижимае к правой тороне
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            // ImGui::SetCursorPosX(
            //     ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
            //     ImGui::CalcTextSize(std::to_string(employees[i].salary).c_str())
            //         .x -
            //     ImGui::GetStyle().ItemSpacing.x);
            char buf_salary[32];
            snprintf(buf_salary, sizeof(buf_salary), "%.2f",
                     employees[i].salary);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                                 ImGui::GetContentRegionAvail().x -
                                 ImGui::CalcTextSize(buf_salary).x);
            ImGui::Text("%0.2f", employees[i].salary);
            ImGui::PopStyleVar();
            // ставка
            ImGui::TableSetColumnIndex(4);
            // прижимае к правой тороне
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            // ImGui::SetCursorPosX(
            //     ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
            //     ImGui::CalcTextSize(std::to_string(employees[i].rate).c_str())
            //         .x -
            //     ImGui::GetStyle().ItemSpacing.x);
            char buf_rate[32];
            snprintf(buf_rate, sizeof(buf_rate), "%.2f", employees[i].rate);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                                 ImGui::GetContentRegionAvail().x -
                                 ImGui::CalcTextSize(buf_rate).x);
            ImGui::Text("%0.2f", employees[i].rate);
            ImGui::PopStyleVar();
            // Отделение
            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%s", employees[i].division.c_str());
            // контракт
            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%s", employees[i].contract.c_str());
            // контракт найден
            ImGui::TableSetColumnIndex(7);
            ImGui::Text("%s", employees[i].contract_found ? "+" : " ");
            // сертификат найден
            ImGui::TableSetColumnIndex(8);
            ImGui::Text("%s", employees[i].certificate_found ? "+" : " ");
            // примечание
            ImGui::TableSetColumnIndex(9);
            // обрабатываем многострочку - просто срезаем после возврата строки
            ImGui::Text("%s", employees[i]
                                  .note.substr(0, employees[i].note.find("\n"))
                                  .c_str());
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

    // std::cout << ". ";
    ImGui::EndChild();
}
