
#include "DivisionsPanel.h"
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

DivisionsPanel::DivisionsPanel(Database &db)
    : Panel("Справочник отделений"),
      db(db) {
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}
DivisionsPanel::~DivisionsPanel() { writeToDatabase(); }

bool DivisionsPanel::writeToDatabase() {
    // Созранение записи из редактора с проверкой изменения
    if (isCurrentChanged()) {
        const bool saved = db.ExecutePrepared(
            "UPDATE Divisions SET division_name = ?, note = ? WHERE id = ?;",
            {currentRecord.division_name, currentRecord.note,
             currentRecord.id});
        if (saved)
            UpdateItemById(divisions, currentRecord);
        return saved;
    }

    return false;
}

bool DivisionsPanel::addRecord() {
    // добавление новой записи в базу
    return db.ExecutePrepared("INSERT INTO Divisions (division_name) VALUES "
                              "(?);",
                              {std::string("")});
}

bool DivisionsPanel::delRecord() {
    // удаление текущей записи
    // хз надо ли контролить зависимые записи или пусть пустыми остаются
    if (currentRecord.id >= 0) {
        return db.ExecutePrepared("DELETE FROM Divisions WHERE id = ?;",
                                  {currentRecord.id});
    }

    return false;
}

void DivisionsPanel::refresh() {
    divisions.clear();
    // Загружаем данные из БД (упрощенный пример)
    const char *sql = "SELECT id, division_name, note FROM Divisions;";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<Division> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                Division{std::stoi(argv[0]), argv[1], argv[2] ? argv[2] : ""});
            return 0;
        },
        &divisions, nullptr);
}

bool DivisionsPanel::isCurrentChanged() {

    if (currentRecord.id < 0)
        return false;

    int currentIndex = FindIndexById(divisions, currentRecord.id);
    if (currentIndex < 0)
        return false;

    // std::cout << " тест " << str << std::endl;
    // std::cout << " новое  :" << currentRecord.note << std::endl;
    // срапвниваем поля
    if (currentRecord.division_name != divisions[currentIndex].division_name)
        return true;
    if (currentRecord.note != divisions[currentIndex].note)
        return true;

    return false;
}

void DivisionsPanel::render() {
    if (!isOpen)
        return;
    // проверка на существование таблицы - вдруг база пауста или не та
    if (!db.tableExists("Divisions")) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Табилца отделений отсуствует!");
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
                          ImVec4(0.2f, 0.7f, 0.9f, 0.6f)); // голубой
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
                          ImVec4(0.2f, 0.7f, 0.2f, 0.6f)); // Зеленый
    if (ImGui::Button(ICON_FA_PLUS)) {
        writeToDatabase();
        if (addRecord()) {
            const int newId =
                static_cast<int>(sqlite3_last_insert_rowid(db.getHandle()));
            refresh();
            SelectItemById(divisions, newId, currentRecord, selectedIndex);
            oldIndex = selectedIndex;
            goBottom = true;
            focusFirst = true;
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Добавить новое отделение");
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    // удаление
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.9f, 0.1f, 0.1f, 0.6f)); // красный
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
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Внимание!");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1, 0, 0, 0.6f),
                           "Удаление выбранного отделения!");
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

    ImGui::Text("Отделение:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(
        ImGui::GetColumnWidth()); // Растянуть на всю колонку
    ImGui::InputText("##Отделение", &currentRecord.division_name);
    // ImGui::InputText("Примечание", &currentRecord.note);

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

    if (ImGui::BeginTable("Divisions", 3,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn("ID",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                50.0f);
        ImGui::TableSetupColumn("Отделение");
        ImGui::TableSetupColumn("Примечание");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < divisions.size(); ++i) {

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            // выделение всей    строки
            bool isSelected = currentRecord.id == divisions[i].id;
            if (ImGui::Selectable(std::to_string(divisions[i].id).c_str(),
                                  isSelected,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                const int selectedId = divisions[i].id;
                writeToDatabase();
                SelectItemById(divisions, selectedId, currentRecord,
                               selectedIndex);
                oldIndex = selectedIndex;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", divisions[i].division_name.c_str());
            ImGui::TableSetColumnIndex(2);
            // обрабатываем многострочку - просто срезаем после возврата строки
            ImGui::Text("%s", divisions[i]
                                  .note.substr(0, divisions[i].note.find("\n"))
                                  .c_str());
            // Прокручиваем к последнему элементу если выделена последняя строка
            // - для добавленной записи
            if (goBottom && i == divisions.size() - 1) {
                ImGui::SetScrollHereY(1.0f); // 1.0f = нижний край экрана
                goBottom = false;
            }
        }
        ImGui::EndTable();
    }

    ImGui::EndChild();
}
