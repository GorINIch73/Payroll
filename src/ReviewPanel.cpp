
#include "ReviewPanel.h"
#include "Icons.h"
#include "imgui_stdlib.h"
#include <algorithm>
// #include <cfloat>
// #include <charconv>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <functional>
#include <imgui.h>
#include <iostream>
// #include <ostream>
#include <string>
// #include <unicode/uchar.h> // Для ICU библиотеки
#include <unicode/utf8.h>

#include "imgui_components.h"

#include <sstream>

// #define CLIP_WIC_SUPPORT false
// #define CLIP_ENABLE_IMAGE false 
// #include <clip.h> 


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
            // std::cout << " add " << text << std::endl;
            row.columns.push_back(text ? text : "NULL");
        }
        data.push_back(row);
    }

    sqlite3_finalize(stmt);
    std::cout << "refresh SQL query" << std::endl;
}

// Функция для копирования выделенных данных в буфер обмена
void ReviewPanel::CopySelectionToClipboard() {

    int min_col, max_col, min_row, max_row;
    // если выделена только одна ячейка
    if (selection_start.x !=-1 && selection_end.x == -1) {
        ImGui::SetClipboardText(data[selection_start.y].columns[selection_start.x].c_str());
        return;
    }

    // если диапазона нет
    if (selection_start.x == -1 || selection_end.x == -1) {
        // если нет выделения копируем все
        min_col = 0;
        max_col = data[0].columns.size()-1;
        min_row = 0;
        max_row = data.size()-1;
    }
    else {

        // Определяем границы выделенной области
        min_col = std::min(selection_start.x, selection_end.x);
        max_col = std::max(selection_start.x, selection_end.x);
        min_row = std::min(selection_start.y, selection_end.y);
        max_row = std::max(selection_start.y, selection_end.y);
    }

    std::ostringstream clipboard_text;

    // Формируем текст для буфера обмена
    for (int row = min_row; row <= max_row; ++row) {
        for (int col = min_col; col <= max_col; ++col) {
            // Добавляем значение ячейки
            clipboard_text << data[row].columns[col];

            // Добавляем табуляцию (кроме последнего столбца)
            if (col < max_col) {
                clipboard_text << '\t';
            }
        }

        // Добавляем перенос строки (кроме последней строки)
        if (row < max_row) {
            clipboard_text << '\n';
        }
    }

    // Копируем в буфер обмена
    if (!clipboard_text.str().empty()) {
        ImGui::SetClipboardText(clipboard_text.str().c_str());
    }


    // std::string clipboard_text;
    //
    // for (int row = 0; row < data.size(); ++row) {
    //     for (int col = 0; col < data[0].columns.size(); ++col) {
    //          clipboard_text += data[row].columns[col];
    //          if (col < data[row].columns.size()) clipboard_text += "\t";
    //     }
    //     clipboard_text += "\n";
    // }
    //
    // if (!clipboard_text.empty()) {
    //     ImGui::SetClipboardText(clipboard_text.c_str());
    // }
}

float ReviewPanel::CalculateSelectionSum() {
    if (selection_start.x == -1 || selection_end.x == -1)
        return 0.0f;

    float sum = 0.0f;
    int cells_counted = 0;

    // Определяем границы выделения
    int min_col = std::min(selection_start.x, selection_end.x);
    int max_col = std::max(selection_start.x, selection_end.x);
    int min_row = std::min(selection_start.y, selection_end.y);
    int max_row = std::max(selection_start.y, selection_end.y);

    // Проходим по всем ячейкам в диапазоне
    for (int row = min_row; row <= max_row; ++row) {
        if (row < 0 || row >= data.size()) continue;
        
        for (int col = min_col; col <= max_col; ++col) {
            if (col < 0 || col >= data[0].columns.size()) continue;

            const std::string& cell_text = data[row].columns[col];
            
            // Пытаемся преобразовать текст в число
            try {
                float value = std::stof(cell_text);
                sum += value;
                cells_counted++;
            } 
            catch (...) {
                // Не числовое значение - пропускаем
                continue;
            }
        }
    }

    return sum;
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



        // Вывод данных в таблицу ImGui
    // if (!data.empty()) {
    //         if (ImGui::BeginTable("Database Table", data[0].columns.size(), ImGuiTableFlags_Borders)) {
    //             // Заголовки столбцов
    //             // for (const auto& columnName : data[0].columns) {
    //             //     ImGui::TableSetupColumn(columnName.c_str());
    //             // }
    //             // ImGui::TableHeadersRow();
    //
    //             for (int i = 0; i < data[0].columns.size();++i) {
    //                 ImGui::TableSetupColumn(std::to_string(i).c_str());
    //             }
    //             ImGui::TableHeadersRow();
    //             // std::cout << "table ... " << std::endl;
    //             // Данные таблицы
    //             for (const auto& row : data) {
    //                 // std::cout << " reow ... " << std::endl;
    //                 ImGui::TableNextRow();
    //                 size_t i =0;
    //                 for (const auto& cell : row.columns) {
    //                     // std::cout << " | ";
    //                     ImGui::TableSetColumnIndex(i);
    //                     // ImGui::TableSetColumnIndex(ImGui::TableGetColumnIndex());
    //                     // std::cout << " " << cell.c_str();
    //                     ImGui::Text("%s", cell.c_str());
    //                     ++i;
    //                 }
    //                     // std::cout << " <> " << std::endl;
    //             }
    //             ImGui::EndTable();
    //         }
    // }


// --------------------------------------------------------------

    column_filter_enabled.resize(data[0].columns.size(), false);
    column_filters.resize(data[0].columns.size(), "");

    // Фильтрация данных
    std::vector<std::vector<std::string>> filtered_rows;
    for (const auto& row : data) {
        bool passes_global_filter = false;
        std::string row_text;
        for (const auto& cell : row.columns) {
            row_text += cell + " ";
        }
        passes_global_filter = global_filter.PassFilter(row_text.c_str());

        bool passes_column_filters = true;
        for (size_t col = 0; col < data[0].columns.size(); ++col) {
            if (column_filter_enabled[col] && !column_filters[col].empty()) {
                if (row.columns[col].find(column_filters[col]) == std::string::npos) {
                    passes_column_filters = false;
                    break;
                }
            }
        }

        if (passes_global_filter && passes_column_filters) {
            filtered_rows.push_back(row.columns);
        }
    }

    // Отображение фильтров
    ImGui::Text("Фильтр:");
    ImGui::SameLine();
    global_filter.Draw("##global_filter");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Глобальный фильтр");
    }
    ImGui::EndGroup();
    ImGui::Separator();

    // Таблица
    if (ImGui::BeginTable("MyTable", data[0].columns.size(), 
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | 
        ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {

        // Заголовки столбцов + фильтры
        for (size_t col = 0; col < data[0].columns.size(); ++col) {
            ImGui::TableSetupColumn(std::to_string(col).c_str());
        }
        ImGui::TableHeadersRow();

        // Фильтры для каждого столбца
        ImGui::TableNextRow();
        for (size_t col = 0; col < data[0].columns.size(); ++col) {
            ImGui::TableSetColumnIndex(col);
            ImGui::PushID(col);

            bool filter_enabled = column_filter_enabled[col];
            if (ImGui::Checkbox("##filter_enabled", &filter_enabled)) {
                column_filter_enabled[col] = filter_enabled;  // Возвращаем значение
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(-FLT_MIN);
            if (ImGui::InputText("##column_filter", &column_filters[col])) {
                // Фильтр обновляется автоматически
            }
            ImGui::PopID();
        }

        // Вывод строк
        for (size_t row = 0; row < filtered_rows.size(); ++row) {
            ImGui::TableNextRow();
            for (size_t col = 0; col < data[0].columns.size(); ++col) {
                ImGui::TableSetColumnIndex(col);
                const auto& cell_value = filtered_rows[row][col];

                // if (col == 0) {
                //     // выделение всей    строки
                //     if (ImGui::Selectable(cell_value.c_str(),
                //                   selectedIndex == row,
                //                   ImGuiSelectableFlags_SpanAllColumns)) {
                //          selectedIndex = row;
                //         std::cout << "selected" << selectedIndex;
                //     }
                // }
                // else  ImGui::Text("%s", cell_value.c_str());


                // --- Проверка, является ли ячейка частью выделения ---
                bool is_in_range = false;
                if (selection_start.x != -1 && selection_end.x != -1) {
                    // Выделен диапазон (после второго клика)
                    is_in_range = (col >= std::min(selection_start.x, selection_end.x)) && 
                                 (col <= std::max(selection_start.x, selection_end.x)) &&
                                 (row >= std::min(selection_start.y, selection_end.y)) && 
                                 (row <= std::max(selection_start.y, selection_end.y));
                }

                // --- Подсветка ячеек ---
                if (waiting_for_end && row == selection_start.y && col == selection_start.x) {
                    // Особый цвет для начальной ячейки (ожидание второго клика)
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, start_cell_color);
                } 
                else if (is_in_range) {
                    // Стандартный цвет для выделенного диапазона
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_Header));
                }

                // --- 1. Создаем невидимую кнопку для обработки кликов ---
                // ImGui::InvisibleButton("##cell", ImGui::GetContentRegionAvail());

                // --- Отображение текста ячейки ---
                ImGui::Text("%s", cell_value.c_str());
        
                // --- Обработка кликов (правая - меню) ---
                if (ImGui::IsItemHovered()) {
                    // ЛКМ - выделение
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
 

                        if (!waiting_for_end) {
                            // Первый клик - устанавливаем начало
                            selection_start = ImVec2(col, row);
                            waiting_for_end = true;
                            // std::cout << "start:" << col << ":" << row;
                        } else {
                            // Второй клик - устанавливаем конец
                            selection_end = ImVec2(col, row);
                            waiting_for_end = false;
                            // std::cout << "end:" << col << ":" << row << std::endl;
                            
                            // Если кликнули ту же ячейку дважды - сбрасываем
                            if (selection_start.x == selection_end.x && 
                                selection_start.y == selection_end.y) {
                                selection_start = ImVec2(-1, -1);
                                selection_end = ImVec2(-1, -1);
                            }
                        }

                    }
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                        ImGui::OpenPopup("CellContextMenu"); // Открываем меню
                    }
                }
            }
        

        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            waiting_for_end = false;
            selection_start = ImVec2(-1, -1);
            selection_end = ImVec2(-1, -1);
        }
        if (waiting_for_end) {
            ImGui::SetTooltip("Кликните конечную ячейку для выделения диапазона");
        }
        // Контекстное меню для копирования
        if (ImGui::BeginPopupContextWindow("CellContextMenu")) {
            if (ImGui::MenuItem("Копировать данные", nullptr, false)) {
                CopySelectionToClipboard();
            }

            float sum = CalculateSelectionSum();
            if (sum != 0.0f) {
                ImGui::Separator();
                ImGui::Text("Сумма: %.2f", sum);
                if (ImGui::MenuItem(" Копировать сумму")) {
                    ImGui::SetClipboardText(std::to_string(sum).c_str());
                }
            }
            ImGui::EndPopup();
        }

        ImGui::EndTable();

        // сброс выделения вне таблицы
        // if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
        //     selection_start = ImVec2(-1, -1);
        //     selection_end = ImVec2(-1, -1);
        //     waiting_for_end = false;
        // }
    }

        //    // Копирование по Ctrl+C
        // if (ImGui::IsKeyPressed(ImGuiKey_C) && ImGui::GetIO().KeyCtrl) {
    ImGui::EndChild();
}
