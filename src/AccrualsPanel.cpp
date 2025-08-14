
#include "AccrualsPanel.h"
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
#include "imgui_components.h"
#include <unicode/utf8.h>

AccrualsPanel::AccrualsPanel(Database &db)
    : Panel("Справочник начислений"),
      db(db) {
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}
AccrualsPanel::~AccrualsPanel() { writeToDatabase(); }

bool AccrualsPanel::writeToDatabase() {
    // Созранение записи из редактора с проверкой изменения
    std::string sql;
    if (isCurrentChanged()) {
        sql = "UPDATE Accruals SET name='" + currentRecord.name +

              "', percentage=" + std::to_string(currentRecord.percentage) +

              ", this_salary=" +
              std::to_string(currentRecord.this_salary ? 1 : 0) + ", note='" +
              currentRecord.note +
              "' WHERE id=" + std::to_string(currentRecord.id) + ";";

        // std::cout << currentRecord.note << std::endl;
        // std::cout << sql << std::endl;

        printf("Обновление записи ... \n");
        return db.Execute(sql);
    }

    return false;
}

bool AccrualsPanel::addRecord() {
    // добавление новой записи в базу
    std::string sql;
    sql = "INSERT INTO Accruals (name) VALUES ( '');";
    return db.Execute(sql);
}

bool AccrualsPanel::delRecord() {
    // удаление текущей записи
    // хз надо ли контролить зависимые записи или пусть пустыми остаются
    if (currentRecord.id >= 0) {
        std::string sql;
        sql = "DELETE FROM Accruals WHERE id =" +
              std::to_string(currentRecord.id) + ";";
        return db.Execute(sql);
    }

    return false;
}

void AccrualsPanel::refresh() {
    accruals.clear();
    // Загружаем данные из БД (упрощенный пример)
    const char *sql =
        "SELECT id, name, percentage, this_salary, note FROM Accruals;";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<Accrual> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(Accrual{
                std::stoi(argv[0]), argv[1] ? argv[1] : "",
                argv[2] ? std::stod(argv[2]) : 0,
                argv[3] ? (std::stoi(argv[3]) > 0 ? true : false) : false,
                argv[4] ? argv[4] : ""});
            return 0;
        },
        &accruals, nullptr);
}

bool AccrualsPanel::isCurrentChanged() {

    // если индексы не определены
    if (currentRecord.id < 0)
        return false;
    if (oldIndex < 0)
        return false;

    // std::cout << " тест " << str << std::endl;
    // std::cout << " новое  :" << currentRecord.note << std::endl;
    // std::cout << " старое :" << accruals[oldIndex].note << std::endl;

    // срапвниваем поля
    if (currentRecord.name != accruals[oldIndex].name)
        return true;
    if (currentRecord.percentage != accruals[oldIndex].percentage)
        return true;
    if (currentRecord.this_salary != accruals[oldIndex].this_salary)
        return true;
    if (currentRecord.note != accruals[oldIndex].note)
        return true;

    return false;
}

void AccrualsPanel::render() {
    if (!isOpen)
        return;
    // проверка на существование таблицы - вдруг база пауста или не та
    if (!db.tableExists("Accruals")) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1),
                           "Табилца начислений отсуствует!");
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
        selectedIndex = accruals.size() - 1;
        goBottom = true;
        focusFirst = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Добавить новое начисление");
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
                           "Удаление выбранного начисления!");
        // ImGui::BulletText("Удаление выбранного сотрудника");
        ImGui::EndTooltip();
    }

    // }
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

    ImGui::Text("Начисление:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(
        ImGui::GetColumnWidth()); // Растянуть на всю колонку
    ImGui::InputText("##Начисление", &currentRecord.name);
    // ImGui::InputText("Примечание", &currentRecord.note);

    ImGui::Text("Процент от оклада:");
    ImGui::SameLine();
    ImGui::SetCursorPosX(
        ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
        ImGui::CalcTextSize(std::to_string(currentRecord.percentage).c_str())
            .x);
    ImGui::SetNextItemWidth(
        ImGui::CalcTextSize(std::to_string(currentRecord.percentage).c_str())
            .x);
    ImGui::InputDouble("##процент", &currentRecord.percentage, 0, 0, "%0.0f");

    ToggleButton("Это оклад:", currentRecord.this_salary);

    // мультистрочник - морока прям
    ImGui::Text("Примечание:");
    // Поле с автопереносом
    InputTextWrapper("##kh", currentRecord.note,
                     ImGui::GetContentRegionAvail().x);
    //
    // Таблица со списком

    // ImGui::SameLine();
    // ImGui::SeparatorText("справочник");

    // ImGui::PushStyleColor(ImGuiCol_Separator,
    //                       ImVec4(1, 0, 0, 1)); // Красный цвет
    ImGui::Separator();
    // ImGui::PopStyleColor();

    if (ImGui::BeginTable("Accruals", 5,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn("ID",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                50.0f);
        ImGui::TableSetupColumn("Начисление");
        ImGui::TableSetupColumn("Процент",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                100.0f);
        ImGui::TableSetupColumn("это оклад",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                10.0f);
        ImGui::TableSetupColumn("Примечание");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < accruals.size(); ++i) {

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            // выделение всей    строки
            if (ImGui::Selectable(std::to_string(accruals[i].id).c_str(),
                                  selectedIndex == i,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                selectedIndex = i;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", accruals[i].name.c_str());

            // доля
            ImGui::TableSetColumnIndex(2);
            // прижимае к правой тороне
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
                ImGui::CalcTextSize(
                    std::to_string(accruals[i].percentage).c_str())
                    .x -
                ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%0.0f", accruals[i].percentage);
            ImGui::PopStyleVar();

            // зависимость от времени
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", accruals[i].this_salary ? "+" : " ");

            ImGui::TableSetColumnIndex(4);
            // обрабатываем многострочку - просто срезаем после возврата строки
            ImGui::Text("%s", accruals[i]
                                  .note.substr(0, accruals[i].note.find("\n"))
                                  .c_str());
            // выделена другая строка
            if (oldIndex != selectedIndex) {
                // записать старые данные
                if (writeToDatabase()) {
                    // Обновляем строку таблицы новыми данными
                    accruals[oldIndex] = currentRecord;
                    // printf("Запись обновлена\n");
                }
                // printf("старый указатель %i, новый указатель %i \n",
                // oldIndex,
                //        selectedEmployee);
                // взять в редактор новые данные
                currentRecord = accruals[selectedIndex];
                oldIndex = selectedIndex;
            }
            // Прокручиваем к последнему элементу если выделена последняя строка
            // - для добавленной записи
            if (goBottom && i == accruals.size() - 1) {
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
