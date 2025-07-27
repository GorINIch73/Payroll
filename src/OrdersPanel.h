#pragma once
#include "Database.h"
#include "Panel.h"
#include <string>
#include <utility>
#include <vector>

struct Order {
        int id = -1;
        std::string number = "";
        std::string date = "2000.01.01";
        bool  found = false;
        bool  protocol_found = false;
        std::string note = "";
};

class OrdersPanel : public Panel {
    public:
        OrdersPanel(Database &db);
        ~OrdersPanel();
        void render() override;
        void refresh();
        bool writeToDatabase();
        bool addRecord();
        bool delRecord();
        bool isCurrentChanged();

    private:
        Database &db;
        std::vector<Order> orders;
        int selectedIndex = -1;
        Order currentRecord;
        int oldIndex = -1; // для отслеживание изменения выделенной строки
};
