
#include "StatementsPanel.h"
#include "Panel.h"
#include "ReviewPanel.h"
// #include "PositionsPanel.h"
// #include "IndividualsPanel.h"
// #include "DivisionsPanel.h"
#include "EmployeesPanel.h"
#include "Icons.h"
#include "imgui_stdlib.h"
#include "Manager.h"
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

StatementsPanel::StatementsPanel(Database &db)
    : Panel("Расчетные ведомости"),
      db(db) {
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}

StatementsPanel::~StatementsPanel() { writeToDatabase(); }

bool StatementsPanel::writeToDatabase() {
    // Созранение записи из редактора с проверкой изменения
    std::string sql;

    if (isCurrentChanged()) {
        sql =
            "UPDATE Statements SET  month=" + std::to_string(currentRecord.month) +
            (currentRecord.employee_id > 0
                 ? ", employee_id=" +
                       std::to_string(currentRecord.employee_id)
                 : "") +
            ", hours_worked=" + std::to_string(currentRecord.hours_worked) + 
            ", timesheet_verified=" +
            std::to_string(currentRecord.timesheet_verified ? 1 : 0) +
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

bool StatementsPanel::addRecord() {
    // добавление новой записи в базу
    std::string sql;
    sql = "INSERT INTO Statements (month) VALUES (" + 
        (currentRecord.month > 0
                 ? std::to_string(currentRecord.month)
                 : "0") +

        ");";
    return db.Execute(sql);
}

bool StatementsPanel::delRecord() {
    // удаление текущей записи
    if (currentRecord.id >= 0) {
        std::string sql;
        sql = "DELETE FROM Statements WHERE id =" +
              std::to_string(currentRecord.id) + ";";
        return db.Execute(sql);
    }

    return false;
}

void StatementsPanel::refresh() {

    // Основная таблица расчетных ведомостей
    statements.clear();
    std::cout << "рефреш ..." << std::endl;
    // Загружаем данные из БД
    const char *sql = "SELECT s.id, s.month, e.id, i.full_name || '-' || p.job_title || '-' || d.division_name || "
        "'-' || e.contract, printf('%.2f', p.salary), e.rate, p.norm, s.hours_worked,"
        "s.timesheet_verified, s.note FROM Statements s LEFT JOIN Employees e ON s.employee_id=e.id "
        "LEFT JOIN Individuals i ON e.individual_id= i.id LEFT JOIN Positions p ON e.position_id = "
        "p.id LEFT JOIN Divisions d ON e.division_id = d.id";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<Statement> *>(data);
            list->emplace_back(Statement{
                std::stoi(argv[0]),
                argv[1] ? std::stoi(argv[1]) : 0,
                argv[2] ? std::stoi(argv[2]) : -1,
                argv[3] ? argv[3] : "",
                argv[4] ? std::stod(argv[4]) : 0,
                argv[5] ? std::stod(argv[5]) : 0,
                argv[6] ? std::stod(argv[6]) : 0,
                argv[7] ? std::stod(argv[7]) : 0,
                argv[8] ? (std::stoi(argv[8]) > 0 ? true : false) : false,
                argv[9] ? argv[9] : "",
            });
            return 0;
        },
        &statements, nullptr);

    // std::cout << "рефреш основной " << std::endl;


    // Основная таблица списка начислений
    list_accruals.clear();
    std::cout << "рефреш ..." << std::endl;
    // Загружаем данные из БД
    const char *sqlL = "SELECT l.id, l.statement_id, l.accrual_id, a.name, a.percentage, a.this_salary, l.amount,"
            "l.order_id, o.number, l.verified, l.note FROM List_accruals l LEFT JOIN Accruals a "
            "ON l.accrual_id=a.id LEFT JOIN Orders o ON l.order_id= o.id";

    sqlite3_exec(
        db.getHandle(), sqlL,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<List_accrual> *>(data);
            list->emplace_back(List_accrual{
                std::stoi(argv[0]),
                argv[1] ? std::stoi(argv[1]) : -1,
                argv[2] ? std::stoi(argv[2]) : -1,
                argv[3] ? argv[3] : "",
                argv[4] ? std::stod(argv[4]) : 0,
                argv[5] ? (std::stoi(argv[5]) > 0 ? true : false) : false,
                argv[6] ? std::stod(argv[6]) : 0,
                argv[7] ? std::stoi(argv[7]) : -1,
                argv[8] ? argv[8] : "",
                argv[9] ? (std::stoi(argv[9]) > 0 ? true : false) : false,
                argv[10] ? argv[10] : "",
            });
            return 0;
        },
        &list_accruals, nullptr);



    // таблица сотрудников
    employees.clear();
    const char *sqlE = "SELECT e.id, i.full_name || '-' || p.job_title || '-' || "
            "d.division_name || '-' || e.contract || '-' || printf('%.2f', p.salary) || "
            "'-' || e.rate || '-' || p.norm FROM Employees e  LEFT JOIN Individuals i ON "
            "e.individual_id= i.id LEFT JOIN Positions p ON e.position_id = p.id LEFT JOIN "
            "Divisions d ON e.division_id = d.id;";
    sqlite3_exec(
        db.getHandle(), sqlE,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<ComboItem> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                ComboItem{std::stoi(argv[0]), argv[1] ? argv[1] : ""});
            return 0;
        },
        &employees, nullptr);

    // std::cout << "рефреш физлиц норм " << std::endl;
    // таблица начислений
    accruals.clear();
    const char *sqlA =
        "SELECT id, name || '-' || printf('%0.0f', percentage) "
        "FROM Accruals;";
    sqlite3_exec(
        db.getHandle(), sqlA,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<ComboItem> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                ComboItem{std::stoi(argv[0]), argv[1] ? argv[1] : ""});
            return 0;
        },
        &accruals, nullptr);

    // std::cout << "рефреш должностей норм" << std::endl;
    
    // таблица приказов
    orders.clear();
    const char *sqlO =
        "SELECT id, number || '-' || date"
        "FROM Odrers;";
    sqlite3_exec(
        db.getHandle(), sqlO,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<ComboItem> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                ComboItem{std::stoi(argv[0]), argv[1] ? argv[1] : ""});
            return 0;
        },
        &orders, nullptr);

    // std::cout << "рефреш отделений норм" << std::endl;
}

bool StatementsPanel::isCurrentChanged() {

    // если индексы не определены
    if (currentRecord.id < 0)
        return false;
    if (oldIndex < 0)
        return false;

    // std::cout << " тест " << str << std::endl;
    // std::cout << " новое  :" << currentRecord.note << std::endl;
    // std::cout << " старое :" << statements[oldIndex].note << std::endl;

    // срапвниваем поля
    if (currentRecord.month != statements[oldIndex].month)
        return true;
    if (currentRecord.employee_id != statements[oldIndex].employee_id)
        return true;
    if (currentRecord.hours_worked != statements[oldIndex].hours_worked)
        return true;
    if (currentRecord.timesheet_verified != statements[oldIndex].timesheet_verified)
        return true;
    if (currentRecord.note != statements[oldIndex].note)
        return true;

    return false;
}

void StatementsPanel::render() {
    if (!isOpen)
        return;
    // проверка на существование таблицы - вдруг база пауста или не та
    if (!db.tableExists("Statements")) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1),
                           "Табилца ведомостей отсуствует!");
        return;
    }

    //    ImGui::Begin(name.c_str(), &isOpen);
    bool goBottom = false;
    bool focusFirst = false;


    const char* months[] = { 
        "Январь", "Февраль", "Март", "Апрель", "Май", "Июнь",
        "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь" 
    };



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
        selectedIndex = statements.size() - 1;
        goBottom = true;
        focusFirst = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Добавить новую запись");
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    // удаление
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.9f, 0.1f, 0.1f, 1.0f)); // красный
    if (ImGui::Button(ICON_FA_TRASH)) {                    /* ... */
        if(selectedIndex>=0)
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
                           "Удаление выбранной записи!");
        // ImGui::BulletText("Удаление выбранного сотрудника");
        ImGui::EndTooltip();
    }
    
    // Глобальный фильтр
    ImGui::SameLine();
    ImGui::Text("Фильтр:");
    ImGui::SameLine();
    global_filter.Draw("##global_filter", ImGui::GetContentRegionAvail().x-100);
    // global_filter.Draw("##global_filter", ImGui::GetContentRegionAvail().x*0.8f);
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
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.2f, 0.7f, 0.5f, 1.0f)); // 
    if (ImGui::Button(ICON_FA_LIST)) {
        

    // Добавляем пнель запроса
    auto newPanel = std::make_unique<ReviewPanel>(db,"SELECT e.id, i.full_name, p.job_title, e.rate, e.contract,"
                                                  "e.contract_found, e.certificate_found, e.note FROM  Statements e "
                                                  "LEFT JOIN Individuals i ON e.individual_id = i.id LEFT JOIN Positions p ON e.position_id = p.id");
    // auto newPanel = std::make_unique<PositionsPanel>(db);
    manager_panels.addPanel(std::move(newPanel));
    manager_panels.getNextEnd()=true;
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

    // месяц


    ImGui::Text("Месяц:");
    ImGui::SameLine();
    // ImGui::InputInt("##месяц_", &currentRecord.month);

    int current_month = currentRecord.month -1; // 1 = Январь, 12 = Декабрь
    
    if (ImGui::Combo("##Месяц", &current_month, months, IM_ARRAYSIZE(months))) {
        currentRecord.month = current_month+1;    
    }

    ImGui::Text("Сотрудник:");
    ImGui::SameLine();
    // открыть панель физлиц
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.2f, 0.5f, 0.2f, 1.0f)); // Зеленый
    if (ImGui::Button(ICON_FA_GROUP)) {
        auto newPanel = std::make_unique<EmployeesPanel>(db);
        manager_panels.addPanel(std::move(newPanel));
        manager_panels.getNextEnd()=true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Сотрудники");
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    // комбобокс с фильтром
    if (ComboWithFilter("##СОТРУДНИКИ", currentRecord.employee_id, employees)) {
        // Обработка изменения выбора
        auto it = std::find_if(employees.begin(), employees.end(),
                               [&](const ComboItem &e) {
                                   return e.id == currentRecord.employee_id;
                               });

        if (it != employees.end()) {
            // ImGui::Text("Выбрано: %s (ID: %d)", it3->name.c_str(),
            currentRecord.employee_id = it->id;
        }
    }


    ImGui::Text("Оклад по должности:");
    ImGui::SameLine();
    ImGui::Text("ID %0.2f",currentRecord.salary);
    // ImGui::SetCursorPosX(
    //     ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
    //     ImGui::CalcTextSize(std::to_string(currentRecord.rate).c_str()).x);
    // ImGui::SetNextItemWidth(
    //     ImGui::CalcTextSize(std::to_string(currentRecord.rate).c_str()).x);
    // ImGui::InputDouble("##ставки", &currentRecord.rate, 0, 0, "%0.2f");

    ImGui::Text("Занято ставок:");
    ImGui::SameLine();
    ImGui::Text("%0.3f",currentRecord.rate);
    ImGui::SameLine();
    ImGui::Text("Норма часов:");
    ImGui::SameLine();
    ImGui::Text("%0.2f",currentRecord.norm);
    

    ImGui::Text("Отработано часов:");
    ImGui::SameLine();
    ImGui::InputDouble("##часы", &currentRecord.hours_worked, 0, 0, "%0.2f");

    ToggleButton("Табель проверен:", currentRecord.timesheet_verified);

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

    if (ImGui::BeginTable("Statements", 8,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn("ID",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                50.0f);
        ImGui::TableSetupColumn("Месяц",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                200.0f);
        ImGui::TableSetupColumn("Сотрудник");
        ImGui::TableSetupColumn("Оклад",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                200.0f);
        ImGui::TableSetupColumn("Ставка",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                100.0f);
        ImGui::TableSetupColumn("Часы",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                100.0f);
        ImGui::TableSetupColumn("табель+",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                10.0f);
        ImGui::TableSetupColumn("Примечание");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < statements.size(); ++i) {

            //фильр - определяем нудна ли нам текущая строка для отображения

            // Собираем всю строку в один текст для фильтрации
            // std::string row_text;
            // row_text += std::to_string(statements[i].id) + " ";
            // row_text += statements[i].individual + " ";
            // row_text += statements[i].position + " ";
            // row_text += std::to_string(statements[i].salary) + " ";
            // row_text += std::to_string(statements[i].rate) + " ";
            // row_text += statements[i].division + " ";
            // row_text += statements[i].contract + " ";
            // row_text += statements[i].note;
            // // если не совпадает с фильтром, то пропускаем строку
            // if (!global_filter.PassFilter(row_text.c_str())) {
            //     continue;
            // }

            // фильтр только по сотруднику
            if (!global_filter.PassFilter(statements[i].employee.c_str())) {
                 continue;
            }

            //таблица
            ImGui::TableNextRow();
            // ID
            ImGui::TableSetColumnIndex(0);
            // выделение всей    строки
            if (ImGui::Selectable(std::to_string(statements[i].id).c_str(),
                                  selectedIndex == i,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                selectedIndex = i;
            }

            // месяц
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", statements[i].month > 0 ? months[statements[i].month-1] : "");
            // ImGui::Text("%d", statements[i].month);
            // сотрудник
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", statements[i].employee.c_str());
            // оклад
            ImGui::TableSetColumnIndex(3);
            // прижимае к правой тороне
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
                ImGui::CalcTextSize(std::to_string(statements[i].salary).c_str())
                    .x -
                ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%0.2f", statements[i].salary);
            ImGui::PopStyleVar();
            // ставка
            ImGui::TableSetColumnIndex(4);
            // прижимае к правой тороне
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
                ImGui::CalcTextSize(std::to_string(statements[i].rate).c_str())
                    .x -
                ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%0.2f", statements[i].rate);
            ImGui::PopStyleVar();
            // часы
            ImGui::TableSetColumnIndex(5);
            // прижимае к правой тороне
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
                ImGui::CalcTextSize(std::to_string(statements[i].hours_worked).c_str())
                    .x -
                ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%0.2f", statements[i].hours_worked);
            ImGui::PopStyleVar();

            // табель сверен
            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%s", statements[i].timesheet_verified ? "+" : " ");
            // примечание
            ImGui::TableSetColumnIndex(7);
            // обрабатываем многострочку - просто срезаем после возврата строки
            ImGui::Text("%s", statements[i]
                                  .note.substr(0, statements[i].note.find("\n"))
                                  .c_str());
            // выделена другая строка
            if (oldIndex != selectedIndex) {
                // записать старые данные
                if (writeToDatabase()) {
                    // Обновляем строку таблицы новыми данными
                    statements[oldIndex] = currentRecord;
                    // printf("Запись обновлена\n");
                }
                // printf("старый указатель %i, новый указатель %i \n",
                // oldIndex,
                //        selectedStatement);
                // взять в редактор новые данные
                currentRecord = statements[selectedIndex];
                oldIndex = selectedIndex;
            }
            // Прокручиваем к последнему элементу если выделена последняя строка
            // - для добавленной записи
            if (goBottom && i == statements.size() - 1) {
                ImGui::SetScrollHereY(1.0f); // 1.0f = нижний край экрана
                goBottom = false;
            }
        }
        ImGui::EndTable();
    }

    //    ImGui::End();

    // ImGui::EndChild();

    // ImGui::BeginChild();
    // setStatement(listPanel.GetSelStatement());

    //    ImGui::Begin(name.c_str(), &isOpen);

    // ImGui::BeginChild(name.c_str());

    //    ImGui::End();

    // std::cout << ". ";
    ImGui::EndChild();
}
