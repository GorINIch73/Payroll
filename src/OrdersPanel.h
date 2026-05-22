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

struct OrderAccrualDetail {
        int list_accrual_id = -1;
        std::string full_name = "";
        std::string job_title = "";
        int month = 0;
        std::string accrual = "";
        double percent = 0.0f;
        bool verified = false;
        std::string note = "";
};

class OrdersPanel : public Panel {
    public:
        OrdersPanel(Database &db);
        ~OrdersPanel();
        void render() override;
        void refresh();
        void refreshAccrualDetails();
        bool writeToDatabase();
        bool addRecord();
        bool delRecord();
        bool isCurrentChanged();
        bool saveAccrualDetail(const OrderAccrualDetail &detail);

    private:
        Database &db;
        std::vector<Order> orders;
        std::vector<OrderAccrualDetail> accrualDetails;
        int selectedIndex = -1;
        Order currentRecord;
        int oldIndex = -1; // для отслеживание изменения выделенной строки
};
