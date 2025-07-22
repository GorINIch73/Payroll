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
        
        void refresh();

    private:
        Database &db;
        
        std::vector<TableRow> data;
        
        std::string query;

        // int selectedIndex = -1;
        // int oldIndex = -1; // для отслеживание изменения выделенной строки
};
