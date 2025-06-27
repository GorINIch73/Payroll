
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
            auto *list = static_cast<std::vector<ListCombo> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                ListCombo{std::stoi(argv[0]), argv[1] ? argv[1] : ""});
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
            auto *list = static_cast<std::vector<ListCombo> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                ListCombo{std::stoi(argv[0]), argv[1] ? argv[1] : ""});
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

    ImGui::InputInt("ФИО1", &currentRecord.individual_id);
    ImGui::InputText("ФИО", &currentRecord.individual);

    // static std::vector<std::string> options = {"Вариант A", "Вариант B",
    // "Вариант C"};
    // static int selected_index = 0;

    // std::cout << "комбо старт" << std::endl;

    // Находим индекс выбранного элемента
    size_t current_index = 0;
    for (; current_index < individuals.size(); ++current_index) {
        if (individuals[current_index].id == currentRecord.individual_id) {
            break;
        }
    }

    if (ImGui::BeginCombo("Физические лица",
                          current_index < individuals.size()
                              ? individuals[current_index].value.c_str()
                              : "Не выбрано")) {
        for (size_t i = 0; i < individuals.size(); i++) {
            bool is_selected =
                (currentRecord.individual_id == individuals[i].id);
            if (ImGui::Selectable(individuals[i].value.c_str(), is_selected)) {
                currentRecord.individual_id = individuals[i].id;
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Находим текущее значение для отображения
    // auto it = std::find_if(individuals.begin(), individuals.end(),
    //                        [&](const ListCombo &i) {
    //                            return i.id == currentRecord.individual_id;
    //                        });
    // const char *preview =
    //     it != individuals.end() ? it->value.c_str() : "Не выбрано";
    // if (ImGui::BeginCombo("dd", preview)) {
    //     for (const auto &item : individuals) {
    //         bool is_selected = (currentRecord.individual_id == item.id);
    //         if (ImGui::Selectable(item.value.c_str(), is_selected)) {
    //             currentRecord.individual_id = item.id;
    //         }
    //         if (is_selected) {
    //             ImGui::SetItemDefaultFocus();
    //         }
    //     }
    //
    //     ImGui::EndCombo();
    // }

    // if (ImGui::BeginCombo(
    //         "ФИО3", individuals[currentRecord.individual_id].value.c_str()))
    //         {
    //     for (int i = 0; i < individuals.size(); i++) {
    //         bool is_selected = (currentRecord.individual_id == i);
    //         if (ImGui::Selectable(individuals[i].value.c_str(), is_selected))
    //         {
    //             currentRecord.individual_id = i;
    //         }
    //         if (is_selected) {
    //             ImGui::SetItemDefaultFocus();
    //         }
    //     }
    //     ImGui::EndCombo();
    // }

    // std::cout << "комбо енд" << std::endl;
    // ImGui::InputText("Примечание", &currentRecord.note);

    // мультистрочник - морока прям
    // Добавляем переносы вручную
    std::string result = "";
    float width = ImGui::GetContentRegionAvail().x;
    float current_line_width = 0.0f;

    int i = 0;

    while (i < currentRecord.note.size()) {
        int old_i = i;
        UChar32 c;
        U8_NEXT(currentRecord.note.c_str(), i, currentRecord.note.size(), c);

        // посчитать размер
        std::string char_s = currentRecord.note.substr(old_i, i - old_i);
        float char_width = ImGui::CalcTextSize(char_s.c_str()).x;

        if (current_line_width + char_width > width &&
            currentRecord.note[i] != '\n') {
            result.push_back('\n');

            // std::cout << "i = " << i << std::endl;
            // std::cout << "currentRecord = " << currentRecord.note[i]
            //           << std::endl;
            // std::cout << "width = " << width << std::endl;
            // std::cout << "char_width = " << char_width << std::endl;
            // std::cout << "current_line_width = " << current_line_width
            //           << std::endl;
            current_line_width = 0;
        }
        result.append(char_s);
        current_line_width += char_width;
    }

    std::vector<char> buffer(result.begin(), result.end());
    // buffer.push_back('\0');
    buffer.resize(currentRecord.note.size() + 1024);

    ImGui::PushTextWrapPos(width);
    bool changed =
        ImGui::InputTextMultiline("Примечание", buffer.data(), buffer.size(),
                                  ImVec2(width, ImGui::GetTextLineHeight() * 3),
                                  ImGuiInputTextFlags_NoHorizontalScroll);

    if (changed) {
        // Удаляем добавленные переносы перед сохранением
        currentRecord.note = buffer.data();
        currentRecord.note.erase(std::remove(currentRecord.note.begin(),
                                             currentRecord.note.end(), '\n'),
                                 currentRecord.note.end());
    }

    ImGui::PopTextWrapPos();

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
