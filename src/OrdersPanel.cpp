
#include "OrdersPanel.h"
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

OrdersPanel::OrdersPanel(Database &db)
    : Panel("Справочник приказов"),
      db(db) {
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}
OrdersPanel::~OrdersPanel() { writeToDatabase(); }

bool OrdersPanel::writeToDatabase() {
    // Созранение записи из редактора с проверкой изменения
    std::string sql;
    if (isCurrentChanged()) {
        sql = "UPDATE Orders SET number='" + currentRecord.number +
            "', date='" + currentRecord.date +
            "', found=" + std::to_string(currentRecord.found ? 1 : 0) +
            ", protocol_found=" + std::to_string(currentRecord.protocol_found ? 1 : 0) +
            ", note='" + currentRecord.note +
            "' WHERE id=" + std::to_string(currentRecord.id) + ";";

        // std::cout << currentRecord.note << std::endl;
        // std::cout << sql << std::endl;

        printf("Обновление записи ... \n");
        return db.Execute(sql);
    }

    return false;
}

bool OrdersPanel::addRecord() {
    // добавление новой записи в базу
    std::string sql;
    sql = "INSERT INTO Orders (number) VALUES ( '');";
    return db.Execute(sql);
}

bool OrdersPanel::delRecord() {
    // удаление текущей записи
    // хз надо ли контролить зависимые записи или пусть пустыми остаются
    if (currentRecord.id >= 0) {
        std::string sql;
        sql = "DELETE FROM Orders WHERE id =" +
              std::to_string(currentRecord.id) + ";";
        return db.Execute(sql);
    }

    return false;
}

void OrdersPanel::refresh() {
    orders.clear();
    // Загружаем данные из БД (упрощенный пример)
    const char *sql = "SELECT id, number, date, found, protocol_found, note FROM Orders;";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<Order> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(Order{std::stoi(argv[0]),
                argv[1] ? argv[1] : "",
                argv[2] ? argv[2] : "2000-01-01",
                argv[3] ? (std::stoi(argv[3]) > 0 ? true : false) : false,
                argv[4] ? (std::stoi(argv[4]) > 0 ? true : false) : false,
                argv[5] ? argv[5] : ""});
            return 0;
        },
        &orders, nullptr);
}

bool OrdersPanel::isCurrentChanged() {

    // если индексы не определены
    if (currentRecord.id < 0)
        return false;
    if (oldIndex < 0)
        return false;

    // std::cout << " тест " << str << std::endl;
    // std::cout << " новое  :" << currentRecord.note << std::endl;
    // std::cout << " старое :" << orders[oldIndex].note << std::endl;

    // срапвниваем поля
    if (currentRecord.number != orders[oldIndex].number)
        return true;
    if (currentRecord.date != orders[oldIndex].date)
        return true;
    if (currentRecord.found != orders[oldIndex].found)
        return true;
    if (currentRecord.protocol_found != orders[oldIndex].protocol_found)
        return true;
    if (currentRecord.note != orders[oldIndex].note)
        return true;

    return false;
}

void OrdersPanel::render() {
    if (!isOpen)
        return;
    // проверка на существование таблицы - вдруг база пауста или не та
    if (!db.tableExists("Orders")) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Табилца приказов отсуствует!");
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
        selectedIndex = orders.size() - 1;
        goBottom = true;
        focusFirst = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Добавить новый приказ");
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

    ImGui::Text("Номер приказа:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(
        ImGui::GetColumnWidth()); // Растянуть на всю колонку
    ImGui::InputText("##Номер", &currentRecord.number);
    // ImGui::InputText("Примечание", &currentRecord.note);

    
    if (InputDate("Дата приказа:    ", currentRecord.date)) {
        // Дата была изменена
                
        // IMGUI_LOG("Новая дата: %s", date.c_str());
        // std::cout <<  date << std::endl;
    }
    

    ToggleButton("Приказ найден:", currentRecord.found);
    ImGui::SameLine();
    ToggleButton("Протокол найден:", currentRecord.protocol_found);

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

    if (ImGui::BeginTable("Orders", 6,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn("ID",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                50.0f);
        ImGui::TableSetupColumn("Номер приказа");
        ImGui::TableSetupColumn("Дата приказа",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                150.0f);
        ImGui::TableSetupColumn("найден+",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                10.0f);
        ImGui::TableSetupColumn("протокол+",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                10.0f);
        ImGui::TableSetupColumn("Примечание");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < orders.size(); ++i) {

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            // выделение всей    строки
            if (ImGui::Selectable(std::to_string(orders[i].id).c_str(),
                                  selectedIndex == i,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                selectedIndex = i;
            }
            // номер
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", orders[i].number.c_str());
            // дата
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", orders[i].date.c_str());
            // найден
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", orders[i].found ? "+" : " ");
            // найден протокол
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", orders[i].protocol_found ? "+" : " ");
            // примечание
            ImGui::TableSetColumnIndex(5);
            // обрабатываем многострочку - просто срезаем после возврата строки
            ImGui::Text("%s",
                        orders[i]
                            .note.substr(0, orders[i].note.find("\n"))
                            .c_str());
            // выделена другая строка
            if (oldIndex != selectedIndex) {
                // записать старые данные
                if (writeToDatabase()) {
                    // Обновляем строку таблицы новыми данными
                    orders[oldIndex] = currentRecord;
                    // printf("Запись обновлена\n");
                }
                // printf("старый указатель %i, новый указатель %i \n",
                // oldIndex,
                //        selectedEmployee);
                // взять в редактор новые данные
                currentRecord = orders[selectedIndex];
                oldIndex = selectedIndex;
            }
            // Прокручиваем к последнему элементу если выделена последняя строка
            // - для добавленной записи
            if (goBottom && i == orders.size() - 1) {
                ImGui::SetScrollHereY(1.0f); // 1.0f = нижний край экрана
                goBottom = false;
            }
        }
        ImGui::EndTable();
    }


    ImGui::EndChild();
}
