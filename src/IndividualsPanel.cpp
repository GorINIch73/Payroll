
#include "IndividualsPanel.h"
#include "Icons.h"
#include "imgui_stdlib.h"
#include <cstdio>
#include <imgui.h>
#include <iostream>
#include <ostream>
#include <string>

IndividualsPanel::IndividualsPanel(Database &db)
    : Panel("Справочник физлиц"),
      db(db) {
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}

bool IndividualsPanel::writeToDatabase() {
    // Созранение записи из редактора с проверкой изменения
    std::string sql;
    if (isCurrentChanged()) {
        sql = "UPDATE Individuals SET full_name='" + currentRecord.full_name +
              "', note='" + currentRecord.note +
              "' WHERE id=" + std::to_string(currentRecord.id) + ";";
        return db.execute(sql);
    }

    return false;
}

bool IndividualsPanel::addRecord() {
    // добавление новой записи в базу
    std::string sql;
    sql = "INSERT INTO Individuals (full_name) VALUES ( '');";
    return db.execute(sql);
}

bool IndividualsPanel::delRecord() {
    // удаление текущей записи
    if (currentRecord.id >= 0) {
        std::string sql;
        sql = "DELETE FROM Individuals WHERE id =" +
              std::to_string(currentRecord.id) + ";";
        return db.execute(sql);
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

    // если индексы не опрадалены
    if (currentRecord.id < 0)
        return false;
    if (oldIndex < 0)
        return false;

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
    ImGui::InputText("ФИО", &currentRecord.full_name);
    // ImGui::InputText("Примечание", &currentRecord.note);

    // мультистрочник - морока прям
    // Выделяем буфер с запасом
    const size_t bufSize = currentRecord.note.size() + 1024;
    std::vector<char> buffer(bufSize);
    strcpy(buffer.data(), currentRecord.note.c_str());

    if (ImGui::InputTextMultiline(
            "Примечание", buffer.data(), bufSize,
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2))) {
        // если ишзменен
        currentRecord.note = buffer.data();
    };
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

        ImGui::TableSetupColumn("ID");
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
            ImGui::Text("%s", individuals[i].note.c_str());
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
                    printf("Запись обновлена\n");
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
