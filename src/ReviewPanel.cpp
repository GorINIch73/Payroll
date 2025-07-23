
#include "ReviewPanel.h"
#include "Icons.h"
#include "imgui_stdlib.h"
#include <algorithm>
// #include <cfloat>
// #include <charconv>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <imgui.h>
#include <iostream>
// #include <ostream>
#include <string>
// #include <unicode/uchar.h> // Для ICU библиотеки
#include <unicode/utf8.h>

#include "imgui_components.h"

ReviewPanel::ReviewPanel(Database &db, std::string qr)
    : Panel("Запрос"),
      db(db),
      query(qr){
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}

ReviewPanel::~ReviewPanel() {  


    std::cout << "destroy SQL query" << std::endl;
}


void ReviewPanel::refresh() {

    // Основная таблица
    //
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db.getHandle(), query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }

    int columnCount = sqlite3_column_count(stmt);
    data.clear();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TableRow row;
        for (int i = 0; i < columnCount; ++i) {
            const char* text = (const char*)sqlite3_column_text(stmt, i);
            std::cout << " add " << text << std::endl;
            row.columns.push_back(text ? text : "NULL");
        }
        data.push_back(row);
    }

    sqlite3_finalize(stmt);
    std::cout << "refresh SQL query" << std::endl;
}


void ReviewPanel::render() {
    if (!isOpen)
        return;
    // проверка на существование таблицы - вдруг база пауста или не та
    // if (!db.tableExists("Review")) {
    //     ImGui::TextColored(ImVec4(1, 0, 0, 1),
    //                        "Табилца сотрудников отсуствует!");
    //     return;
    // }

    //    ImGui::Begin(name.c_str(), &isOpen);
    // bool goBottom = false;
    // bool focusFirst = false;

    // std::cout << "render SQL Review." << std::endl;
    
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
    // }
    // ImGui::End();

    ImGui::EndGroup();


        // Вывод данных в таблицу ImGui
    if (!data.empty()) {
            if (ImGui::BeginTable("Database Table", data[0].columns.size(), ImGuiTableFlags_Borders)) {
                // Заголовки столбцов
                // for (const auto& columnName : data[0].columns) {
                //     ImGui::TableSetupColumn(columnName.c_str());
                // }
                // ImGui::TableHeadersRow();

                for (int i = 0; i < data[0].columns.size();++i) {
                    ImGui::TableSetupColumn(std::to_string(i).c_str());
                }
                ImGui::TableHeadersRow();
                // std::cout << "table ... " << std::endl;
                // Данные таблицы
                for (const auto& row : data) {
                    // std::cout << " reow ... " << std::endl;
                    ImGui::TableNextRow();
                    size_t i =0;
                    for (const auto& cell : row.columns) {
                        // std::cout << " | ";
                        ImGui::TableSetColumnIndex(i);
                        // ImGui::TableSetColumnIndex(ImGui::TableGetColumnIndex());
                        // std::cout << " " << cell.c_str();
                        ImGui::Text("%s", cell.c_str());
                        ++i;
                    }
                        // std::cout << " <> " << std::endl;
                }
                ImGui::EndTable();
            }
    }




    ImGui::EndChild();
}
