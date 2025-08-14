
#include "IndividualsPanel.h"
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

IndividualsPanel::IndividualsPanel(Database &db)
    : Panel("Справочник физлиц"),
      db(db) {
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}
IndividualsPanel::~IndividualsPanel() { writeToDatabase(); }

bool IndividualsPanel::writeToDatabase() {
    // Созранение записи из редактора с проверкой изменения
    std::string sql;
    if (isCurrentChanged()) {
        sql = "UPDATE Individuals SET full_name='" + currentRecord.full_name +
              "', note='" + currentRecord.note +
              "' WHERE id=" + std::to_string(currentRecord.id) + ";";

        // std::cout << currentRecord.note << std::endl;
        // std::cout << sql << std::endl;

        printf("Обновление записи ... \n");
        return db.Execute(sql);
    }

    return false;
}

bool IndividualsPanel::addRecord() {
    // добавление новой записи в базу
    std::string sql;
    sql = "INSERT INTO Individuals (full_name) VALUES ( '');";
    return db.Execute(sql);
}

bool IndividualsPanel::delRecord() {
    // удаление текущей записи
    // хз надо ли контролить зависимые записи или пусть пустыми остаются
    if (currentRecord.id >= 0) {
        std::string sql;
        sql = "DELETE FROM Individuals WHERE id =" +
              std::to_string(currentRecord.id) + ";";
        return db.Execute(sql);
    }

    return false;
}

void IndividualsPanel::refresh() {
    individuals.clear();
    // Загружаем данные из БД (упрощенный пример)
    const char *sql = "SELECT id, full_name, note FROM Individuals;";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<Individual> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(Individual{std::stoi(argv[0]), argv[1],
                                          argv[2] ? argv[2] : ""});
            return 0;
        },
        &individuals, nullptr);
}

bool IndividualsPanel::isCurrentChanged() {

    // если индексы не определены
    if (currentRecord.id < 0)
        return false;
    if (oldIndex < 0)
        return false;

    // std::cout << " тест " << str << std::endl;
    // std::cout << " новое  :" << currentRecord.note << std::endl;
    // std::cout << " старое :" << individuals[oldIndex].note << std::endl;

    // срапвниваем поля
    if (currentRecord.full_name != individuals[oldIndex].full_name)
        return true;
    if (currentRecord.note != individuals[oldIndex].note)
        return true;

    return false;
}

void IndividualsPanel::render() {
    if (!isOpen)
        return;
    // проверка на существование таблицы - вдруг база пауста или не та
    if (!db.tableExists("Individuals")) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Табилца физлиц отсуствует!");
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
        selectedIndex = individuals.size() - 1;
        goBottom = true;
        focusFirst = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Добавить новое физлицо");
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

    ImGui::Text("ФИО:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(
        ImGui::GetColumnWidth()); // Растянуть на всю колонку
    ImGui::InputText("##ФИО", &currentRecord.full_name);
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

    if (ImGui::BeginTable("Individuals", 3,
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

        for (size_t i = 0; i < individuals.size(); ++i) {

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            // выделение всей    строки
            if (ImGui::Selectable(std::to_string(individuals[i].id).c_str(),
                                  selectedIndex == i,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                selectedIndex = i;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", individuals[i].full_name.c_str());
            ImGui::TableSetColumnIndex(2);
            // обрабатываем многострочку - просто срезаем после возврата строки
            ImGui::Text("%s",
                        individuals[i]
                            .note.substr(0, individuals[i].note.find("\n"))
                            .c_str());
            // ImGui::TableSetColumnIndex(3);
            // // выравнивание вправо
            // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            // ImGui::SetCursorPosX(
            //     ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
            //     ImGui::CalcTextSize(
            //         std::to_string(individuals[i].salary).c_str())
            //         .x -
            //     ImGui::GetStyle().ItemSpacing.x);
            // ImGui::Text("%.2f", individuals[i].salary);
            // ImGui::PopStyleVar();
            //
            // выделена другая строка
            if (oldIndex != selectedIndex) {
                // записать старые данные
                if (writeToDatabase()) {
                    // Обновляем строку таблицы новыми данными
                    individuals[oldIndex] = currentRecord;
                    // printf("Запись обновлена\n");
                }
                // printf("старый указатель %i, новый указатель %i \n",
                // oldIndex,
                //        selectedEmployee);
                // взять в редактор новые данные
                currentRecord = individuals[selectedIndex];
                oldIndex = selectedIndex;
            }
            // Прокручиваем к последнему элементу если выделена последняя строка
            // - для добавленной записи
            if (goBottom && i == individuals.size() - 1) {
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
