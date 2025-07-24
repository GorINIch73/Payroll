#pragma once
#include "Database.h"
#include "Panel.h"
#include "imgui_components.h"
#include <string>
#include <utility>
#include <vector>

// Структура для хранения данных из БД
struct TableRow {
    std::vector<std::string> columns;
};

// struct ListItem {
//         int id = -1;
//         std::string value = "";
// };
//
class ReviewPanel : public Panel {
    public:
        ReviewPanel(Database &db,std::string qr);
        ~ReviewPanel();
        void render() override;
        void CopySelectionToClipboard();
        float CalculateSelectionSum();
        
        void refresh();

    private:
        Database &db;
        
        std::vector<TableRow> data;
        
        std::string query;
    // std::vector<std::vector<std::string>> rows;
    // std::vector<std::string> headers;
        std::vector<bool> column_filter_enabled;
        std::vector<std::string> column_filters;
        ImGuiTextFilter global_filter;

        int selectedIndex = -1;

        ImVec2 selection_start = {-1, -1};  // Начало выделения
        ImVec2 selection_end = {-1, -1};    // Конец выделения
        bool waiting_for_end = false;       // Ожидаем второй клик?

        ImU32 start_cell_color = ImGui::GetColorU32(ImGuiCol_ButtonHovered); // Цвет начальной ячейк
    
};
