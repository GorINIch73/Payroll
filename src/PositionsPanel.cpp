
#include "PositionsPanel.h"
#include "Icons.h"
#include "imgui_stdlib.h"
#include <algorithm>
// #include <cfloat>
// #include <charconv>
#include <cstdio>
#include <cstring>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
// #include <ostream>
#include <string>
// #include <unicode/uchar.h> // Для ICU библиотеки
#include "imgui_components.h"
#include <unicode/utf8.h>

PositionsPanel::PositionsPanel(Database &db)
    : Panel("Справочник должностей"),
      db(db) {
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}

PositionsPanel::~PositionsPanel() { writeToDatabase(); }

bool PositionsPanel::writeToDatabase() {
    // Созранение записи из редактора с проверкой изменения
    std::string sql;
    if (isCurrentChanged()) {
        sql = "UPDATE Positions SET job_title='" + currentRecord.job_title +
              "', salary=" + std::to_string(currentRecord.salary) +
              ", norm=" + std::to_string(currentRecord.norm) + ", note='" +
              currentRecord.note +
              "' WHERE id=" + std::to_string(currentRecord.id) + ";";

        // std::cout << currentRecord.note << std::endl;
        // std::cout << sql << std::endl;

        printf("Обновление записи ... \n");
        return db.execute(sql);
    }

    return false;
}

bool PositionsPanel::addRecord() {
    // добавление новой записи в базу
    std::string sql;
    sql = "INSERT INTO Positions (job_title) VALUES ( '');";
    return db.execute(sql);
}

bool PositionsPanel::delRecord() {
    // удаление текущей записи
    // хз надо ли контролить зависимые записи или пусть пустыми остаются

    if (currentRecord.id >= 0) {
        std::string sql;
        sql = "DELETE FROM Positions WHERE id =" +
              std::to_string(currentRecord.id) + ";";
        return db.execute(sql);
    }

    return false;
}

void PositionsPanel::refresh() {
    positions.clear();
    // Загружаем данные из БД (упрощенный пример)
    const char *sql =
        "SELECT id, job_title, salary, norm, note FROM Positions;";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<Position> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(Position{
                std::stoi(argv[0]), argv[1] ? argv[1] : "",
                argv[2] ? std::stod(argv[2]) : 0,
                argv[3] ? std::stod(argv[3]) : 1, argv[4] ? argv[4] : ""});
            return 0;
        },
        &positions, nullptr);
}

bool PositionsPanel::isCurrentChanged() {

    // если индексы не определены
    if (currentRecord.id < 0)
        return false;
    if (oldIndex < 0)
        return false;

    // std::cout << " тест " << str << std::endl;
    // std::cout << " новое  :" << currentRecord.note << std::endl;
    // std::cout << " старое :" << positions[oldIndex].note << std::endl;

    // срапвниваем поля
    if (currentRecord.job_title != positions[oldIndex].job_title)
        return true;
    if (currentRecord.salary != positions[oldIndex].salary)
        return true;
    if (currentRecord.norm != positions[oldIndex].norm)
        return true;
    if (currentRecord.note != positions[oldIndex].note)
        return true;

    return false;
}

void PositionsPanel::render() {
    if (!isOpen)
        return;
    // проверка на существование таблицы - вдруг база пауста или не та
    if (!db.tableExists("Positions")) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1),
                           "Табилца должностей отсуствует!");
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
        selectedIndex = positions.size() - 1;
        goBottom = true;
        focusFirst = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Добавить новую должность");
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

    ImGui::Text("Должность:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(
        ImGui::GetColumnWidth()); // Растянуть на всю колонку
    ImGui::InputText("##должность", &currentRecord.job_title);

    // Выравнивание
    // ImGui::SetCursorPosX(
    //     ImGui::GetCursorPosX() + ImGui::CalcItemWidth() -
    //     ImGui::CalcTextSize(std::to_string(currentRecord.salary).c_str()).x -
    //     ImGui::GetStyle().FramePadding.x * 2);

    ImGui::Text("Оклад:");
    ImGui::SameLine();
    // ImGui::SetCursorPosX(
    //     ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
    //     ImGui::CalcTextSize(std::to_string(currentRecord.salary).c_str()).x -
    //     ImGui::GetStyle().ItemSpacing.x);

    ImGui::SetCursorPosX(
        ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
        ImGui::CalcTextSize(std::to_string(currentRecord.salary).c_str()).x);
    ImGui::SetNextItemWidth(
        ImGui::CalcTextSize(std::to_string(currentRecord.salary).c_str()).x);
    ImGui::InputDouble("##оклад", &currentRecord.salary, 0, 0, "%.2f");
    // ImGui::Text(":");
    // ImGui::SameLine();
    // ImGui::InputDouble("##Норма часов", &currentRecord.norm, 0, 0,
    // "%000.3f");
    //
    // ImGui::BeginGroup();
    ImGui::Text("Норма часов:");
    ImGui::SameLine();

    // выравнивание вправо
    // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::SetCursorPosX(
        ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
        ImGui::CalcTextSize(std::to_string(currentRecord.norm).c_str()).x);

    ImGui::SetNextItemWidth(
        ImGui::CalcTextSize(std::to_string(currentRecord.norm).c_str()).x);

    ImGui::InputDouble("##норма", &currentRecord.norm, 0, 0, "%000.3f");

    // ImGui::PopStyleVar();

    // ImGui::EndGroup();

    // double ttt = 0.00f;
    // ImGui::BeginGroup();
    // ImGui::InputDouble("##input", &ttt); // Невидимая метка
    // ImRect rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    //
    // ImGui::SameLine();
    // ImGui::SetCursorPosX(rect.Max.x - ImGui::CalcTextSize("123.45").x);
    // ImGui::Text("%.2f", ttt); // Наложенный текст
    // ImGui::EndGroup();

    // ImGui::InputText("Примечание", &currentRecord.note);

    // мультистрочник - морока прям

    ImGui::Text("Примечание:");
    // Поле с автопереносами
    InputTextWrapper("##kh", currentRecord.note,
                     ImGui::GetContentRegionAvail().x);

    // Таблица со списком

    // ImGui::SameLine();
    // ImGui::SeparatorText("справочник");

    // ImGui::PushStyleColor(ImGuiCol_Separator,
    //                       ImVec4(1, 0, 0, 1)); // Красный цвет
    ImGui::Separator();
    // ImGui::PopStyleColor();

    if (ImGui::BeginTable("Positions", 5,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("ID",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                50.0f);
        ImGui::TableSetupColumn("Должность");
        ImGui::TableSetupColumn("Оклад",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                300.0f);
        ImGui::TableSetupColumn("Норма времени",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                100.0f);
        ImGui::TableSetupColumn("Примечание");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < positions.size(); ++i) {

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            // выделение всей    строки
            if (ImGui::Selectable(std::to_string(positions[i].id).c_str(),
                                  selectedIndex == i,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                selectedIndex = i;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", positions[i].job_title.c_str());
            ImGui::TableSetColumnIndex(2);
            // выравнивание вправо
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
                ImGui::CalcTextSize(std::to_string(positions[i].salary).c_str())
                    .x -
                ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%.2f", positions[i].salary);
            ImGui::PopStyleVar();
            ImGui::TableSetColumnIndex(3);
            // выравнивание вправо
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
                ImGui::CalcTextSize(std::to_string(positions[i].norm).c_str())
                    .x -
                ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%.3f", positions[i].norm);
            ImGui::PopStyleVar();
            ImGui::TableSetColumnIndex(4);
            // обрабатываем многострочку - просто срезаем после возврата строки
            ImGui::Text("%s", positions[i]
                                  .note.substr(0, positions[i].note.find("\n"))
                                  .c_str());
            // ImGui::TableSetColumnIndex(3);
            // // выравнивание вправо
            // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            // ImGui::SetCursorPosX(
            //     ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
            //     ImGui::CalcTextSize(
            //         std::to_string(positions[i].salary).c_str())
            //         .x -
            //     ImGui::GetStyle().ItemSpacing.x);
            // ImGui::Text("%.2f", positions[i].salary);
            // ImGui::PopStyleVar();
            //
            // выделена другая строка
            if (oldIndex != selectedIndex) {
                // записать старые данные
                if (writeToDatabase()) {
                    // Обновляем строку таблицы новыми данными
                    positions[oldIndex] = currentRecord;
                    // printf("Запись обновлена\n");
                }
                // printf("старый указатель %i, новый указатель %i \n",
                // oldIndex,
                //        selectedEmployee);
                // взять в редактор новые данные
                currentRecord = positions[selectedIndex];
                oldIndex = selectedIndex;
            }
            // Прокручиваем к последнему элементу если выделена последняя строка
            // - для добавленной записи
            if (goBottom && i == positions.size() - 1) {
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
