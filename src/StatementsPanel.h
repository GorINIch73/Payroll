#pragma once
#include "Database.h"
#include "Panel.h"
#include "imgui_components.h"
#include <string>
#include <utility>
#include <vector>

// ведомость 
struct Statement {
        int id = -1;
        int month = 0;
        int employee_id = -1;
        std::string employee = "";
        double salary = 0.0f; // оклад по договору только для расчетов
        double rate = 0.0f; // ставка только для расчетов
        double norm = 0.0f; // норма чсот из справочника должностей только для расчетов
        double hours_worked = 0.0f;
        bool timesheet_verified = false;
        std::string note = "";
};

// список начислений для ведомости
struct List_accrual {
        int id = -1;
        int statement_id = -1;
        int accrual_id = -1;
        std::string accrual = "";
        double percentage = 0.0f; // процент для расчетов
        bool this_salary = false; // признак оклада для расчетов
        double amount = 0.0f;
        int order_id = -1;
        std::string order = "";
        bool verified = false;
        std::string note = "";
};

class StatementsPanel : public Panel {
    public:
        StatementsPanel(Database &db);
        ~StatementsPanel();
        void render() override;
        void refresh();
        bool writeToDatabase();
        bool addRecord();
        bool delRecord();
        bool isCurrentChanged();

    private:
        Database &db;
        std::vector<Statement> statements;    // основная таблица расчетных ведомостей
        std::vector<List_accrual> list_accruals;    // основная таблица списка начислений для ведомости
        std::vector<ComboItem> employees; // таблица сотрудников для комбо
        std::vector<ComboItem> accruals;   // таблица видов начислений для комбо
        std::vector<ComboItem> orders;   // таблица приказов для комбо

        ImGuiTextFilter global_filter;  // <-- Глобальный фильтр
        int selectedIndex = -1;
        Statement currentRecord;
        int oldIndex = -1; // для отслеживание изменения выделенной строки
};
