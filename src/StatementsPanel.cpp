
#include "StatementsPanel.h"
#include "AccrualsPanel.h"
#include "OrdersPanel.h"
#include "Panel.h"
#include "ReviewPanel.h"
// #include "PositionsPanel.h"
// #include "IndividualsPanel.h"
// #include "DivisionsPanel.h"
#include "EmployeesPanel.h"
#include "Icons.h"
#include "Manager.h"
#include "imgui_stdlib.h"
#include <algorithm>
// #include <cfloat>
// #include <charconv>
#include <cstdio>
#include <cstring>
#include <imgui.h>
#include <iostream>
// #include <ostream>
#include <memory>
#include <string>
// #include <unicode/uchar.h> // Для ICU библиотеки
#include <unicode/utf8.h>
#include <utility>

#include "imgui_components.h"

namespace {
int compareStatements(const Statement &left, const Statement &right,
                      ImGuiID columnId) {
    switch (columnId) {
    case 0:
        return left.id - right.id;
    case 1:
        return left.month - right.month;
    case 2:
        return left.employee.compare(right.employee);
    case 3:
        return (left.salary > right.salary) - (left.salary < right.salary);
    case 4:
        return (left.rate > right.rate) - (left.rate < right.rate);
    case 5:
        return (left.hours_norm > right.hours_norm) -
               (left.hours_norm < right.hours_norm);
    case 6:
        return (left.hours_worked > right.hours_worked) -
               (left.hours_worked < right.hours_worked);
    case 7:
        return static_cast<int>(left.timesheet_verified) -
               static_cast<int>(right.timesheet_verified);
    case 8:
        return left.note.compare(right.note);
    default:
        return 0;
    }
}

void sortStatements(std::vector<Statement> &statements,
                    const ImGuiTableSortSpecs *sortSpecs) {
    if (!sortSpecs || sortSpecs->SpecsCount == 0)
        return;

    std::stable_sort(
        statements.begin(), statements.end(),
        [sortSpecs](const Statement &left, const Statement &right) {
            for (int i = 0; i < sortSpecs->SpecsCount; ++i) {
                const ImGuiTableColumnSortSpecs &spec = sortSpecs->Specs[i];
                int result = compareStatements(left, right, spec.ColumnUserID);
                if (result == 0)
                    continue;

                return spec.SortDirection == ImGuiSortDirection_Ascending
                           ? result < 0
                           : result > 0;
            }

            return left.id < right.id;
        });
}

template <typename T> int findIndexById(const std::vector<T> &items, int id) {
    auto it = std::find_if(items.begin(), items.end(),
                           [id](const T &item) { return item.id == id; });
    if (it == items.end())
        return -1;

    return static_cast<int>(std::distance(items.begin(), it));
}
} // namespace

StatementsPanel::StatementsPanel(Database &db)
    : Panel("Расчетные ведомости"),
      db(db) {
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}

StatementsPanel::~StatementsPanel() {
    writeToDatabase();
    writeAccrualToDatabase();
}

bool StatementsPanel::writeToDatabase() {
    // Созранение записи из редактора с проверкой изменения
    std::string sql;

    if (isCurrentChanged()) {
        sql =
            "UPDATE Statements SET  month=" +
            std::to_string(currentRecord.month) +
            (currentRecord.employee_id > 0
                 ? ", employee_id=" + std::to_string(currentRecord.employee_id)
                 : "") +
            ", hours_norm=" + std::to_string(currentRecord.hours_norm) +
            ", hours_worked=" + std::to_string(currentRecord.hours_worked) +
            ", timesheet_verified=" +
            std::to_string(currentRecord.timesheet_verified ? 1 : 0) +
            ", note='" + currentRecord.note +
            "' WHERE id=" + std::to_string(currentRecord.id) + ";";

        // std::cout << currentRecord.note << std::endl;
        // std::cout << sql << std::endl;
        if (db.Execute(sql)) {

            // обновляем запись в таблице пока что полным обновлением
            refresh();
        };
        return false;
    }

    return false;
}

bool StatementsPanel::writeAccrualToDatabase() {
    // Сохранение записи из редактора с проверкой изменения для начислений
    std::string sql;

    if (isAccrualCurrentChanged()) {
        sql =
            "UPDATE List_accruals SET  statement_id=" +
            std::to_string(currentAccrualRecord.statement_id) +
            (currentAccrualRecord.accrual_id > 0
                 ? ", accrual_id=" +
                       std::to_string(currentAccrualRecord.accrual_id)
                 : "") +
            ", amount=" + std::to_string(currentAccrualRecord.amount) +
            (currentAccrualRecord.order_id > 0
                 ? ", order_id=" + std::to_string(currentAccrualRecord.order_id)
                 : "") +
            ", verified=" +
            std::to_string(currentAccrualRecord.verified ? 1 : 0) + ", note='" +
            currentAccrualRecord.note +
            "' WHERE id=" + std::to_string(currentAccrualRecord.id) + ";";

        // std::cout << currentRecord.note << std::endl;
        // std::cout << sql << std::endl;
        if (db.Execute(sql)) {

            // обновляем запись в таблице пока что полным обновлением
            refresh();
        };
        return false;
    }

    return false;
}

bool StatementsPanel::addRecord() {
    // добавление новой записи в базу
    std::string sql;
    sql =
        "INSERT INTO Statements (month) VALUES (" +
        (currentRecord.month > 0 ? std::to_string(currentRecord.month) : "0") +

        ");";
    return db.Execute(sql);
}

bool StatementsPanel::addAccrualRecord() {
    // добавление новой записи начисления в базу
    std::string sql;
    if (currentRecord.id >= 0) {
        sql = "INSERT INTO list_accruals (statement_id) VALUES (" +
              std::to_string(currentRecord.id) + ");";
        return db.Execute(sql);
    }
    return false;
}

bool StatementsPanel::delRecord() {
    // удаление текущей записи
    if (currentRecord.id >= 0) {
        std::string sql;
        sql = "DELETE FROM List_accruals WHERE statement_id =" +
              std::to_string(currentRecord.id) + "; " +
              "DELETE FROM Statements WHERE id =" +
              std::to_string(currentRecord.id) + ";";
        return db.Execute(sql);
    }

    return false;
}

bool StatementsPanel::delAccrualRecord() {
    // удаление текущей записи начисления
    if (currentAccrualRecord.id >= 0) {
        std::string sql;
        sql = "DELETE FROM list_accruals WHERE id =" +
              std::to_string(currentAccrualRecord.id) + ";";
        return db.Execute(sql);
    }

    return false;
}

void StatementsPanel::refresh() {
    // Основная таблица расчетных ведомостей
    statements.clear();
    std::cout << "рефреш ..." << std::endl;
    // Загружаем данные из БД
    const char *sql = "SELECT s.id, s.month, e.id, i.full_name || '-' || "
                      "p.job_title || '-' || d.division_name || "
                      "'-' || e.contract, printf('%.2f', p.salary), e.rate, "
                      "p.norm, s.hours_norm, s.hours_worked,"
                      "s.timesheet_verified, s.note FROM Statements s LEFT "
                      "JOIN Employees e ON s.employee_id=e.id "
                      "LEFT JOIN Individuals i ON e.individual_id= i.id LEFT "
                      "JOIN Positions p ON e.position_id = "
                      "p.id LEFT JOIN Divisions d ON e.division_id = d.id";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<Statement> *>(data);
            list->emplace_back(Statement{
                std::stoi(argv[0]),
                argv[1] ? std::stoi(argv[1]) : 0,
                argv[2] ? std::stoi(argv[2]) : -1,
                argv[3] ? argv[3] : "",
                argv[4] ? std::stod(argv[4]) : 0,
                argv[5] ? std::stod(argv[5]) : 0,
                argv[6] ? std::stod(argv[6]) : 0,
                argv[7] ? std::stod(argv[7]) : 0,
                argv[8] ? std::stod(argv[8]) : 0,
                argv[9] ? (std::stoi(argv[9]) > 0 ? true : false) : false,
                argv[10] ? argv[10] : "",
            });
            return 0;
        },
        &statements, nullptr);

    // std::cout << "рефреш основной " << std::endl;

    // Основная таблица списка начислений
    refreshAccruals();

    // таблица сотрудников
    employees.clear();
    const char *sqlE =
        "SELECT e.id, i.full_name || '-' || p.job_title || '-' || "
        "d.division_name || '-' || e.contract || '-' || printf('%.2f', "
        "p.salary) || "
        "'-' || e.rate FROM Employees e  LEFT JOIN Individuals i ON "
        "e.individual_id= i.id LEFT JOIN Positions p ON e.position_id = p.id "
        "LEFT JOIN "
        "Divisions d ON e.division_id = d.id;";
    sqlite3_exec(
        db.getHandle(), sqlE,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<ComboItem> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                ComboItem{std::stoi(argv[0]), argv[1] ? argv[1] : ""});
            return 0;
        },
        &employees, nullptr);

    // std::cout << "рефреш отделений норм" << std::endl;
}

void StatementsPanel::refreshAccruals() {

    // Основная таблица списка начислений
    list_accruals.clear();
    std::cout << "рефреш ..." << std::endl;
    // Загружаем данные из БД
    const char *sqlL =
        "SELECT l.id, l.statement_id, l.accrual_id, a.name, a.percentage, "
        "a.this_salary, l.amount,"
        "l.order_id, o.number || '-' || o.date , l.verified, a.verification, "
        "l.note FROM "
        "List_accruals l LEFT JOIN Accruals a "
        "ON l.accrual_id=a.id LEFT JOIN Orders o ON l.order_id= o.id";

    sqlite3_exec(
        db.getHandle(), sqlL,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<List_accrual> *>(data);
            list->emplace_back(List_accrual{
                std::stoi(argv[0]),
                argv[1] ? std::stoi(argv[1]) : -1,
                argv[2] ? std::stoi(argv[2]) : -1,
                argv[3] ? argv[3] : "",
                argv[4] ? std::stod(argv[4]) : 0,
                argv[5] ? (std::stoi(argv[5]) > 0 ? true : false) : false,
                argv[6] ? std::stod(argv[6]) : 0,
                argv[7] ? std::stoi(argv[7]) : -1,
                argv[8] ? argv[8] : "",
                argv[9] ? (std::stoi(argv[9]) > 0 ? true : false) : false,
                argv[10] ? (std::stoi(argv[10]) > 0 ? true : false) : false,
                argv[11] ? argv[11] : "",
            });
            return 0;
        },
        &list_accruals, nullptr);

    // std::cout << "рефреш физлиц норм " << std::endl;
    // таблица начислений
    accruals.clear();
    const char *sqlA = "SELECT id, name || '-' || printf('%0.0f', percentage) "
                       "FROM Accruals;";
    sqlite3_exec(
        db.getHandle(), sqlA,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<ComboItem> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                ComboItem{std::stoi(argv[0]), argv[1] ? argv[1] : ""});
            return 0;
        },
        &accruals, nullptr);

    // std::cout << "рефреш должностей норм" << std::endl;

    // таблица приказов
    orders.clear();
    const char *sqlO = "SELECT id, number || '-' || date "
                       "FROM Orders;";
    sqlite3_exec(
        db.getHandle(), sqlO,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<ComboItem> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                ComboItem{std::stoi(argv[0]), argv[1] ? argv[1] : ""});
            return 0;
        },
        &orders, nullptr);

    // std::cout << "рефреш отделений норм" << std::endl;
}

bool StatementsPanel::isCurrentChanged() {

    if (currentRecord.id < 0)
        return false;

    const int currentIndex = findIndexById(statements, currentRecord.id);
    if (currentIndex < 0)
        return false;

    // срапвниваем поля
    if (currentRecord.month != statements[currentIndex].month)
        return true;
    if (currentRecord.employee_id != statements[currentIndex].employee_id)
        return true;
    if (currentRecord.hours_norm != statements[currentIndex].hours_norm)
        return true;
    if (currentRecord.hours_worked != statements[currentIndex].hours_worked)
        return true;
    if (currentRecord.timesheet_verified !=
        statements[currentIndex].timesheet_verified)
        return true;
    if (currentRecord.note != statements[currentIndex].note)
        return true;

    return false;
}

bool StatementsPanel::isAccrualCurrentChanged() {

    if (currentAccrualRecord.id < 0)
        return false;
    if (currentRecord.id < 0)
        return false; // если не выделена ведомость ничего не делаем

    const int currentIndex =
        findIndexById(list_accruals, currentAccrualRecord.id);
    if (currentIndex < 0)
        return false;

    // сравниваем поля
    if (currentAccrualRecord.statement_id !=
        list_accruals[currentIndex].statement_id)
        return true;
    if (currentAccrualRecord.accrual_id !=
        list_accruals[currentIndex].accrual_id)
        return true;
    if (currentAccrualRecord.amount != list_accruals[currentIndex].amount)
        return true;
    if (currentAccrualRecord.order_id !=
        list_accruals[currentIndex].order_id)
        return true;
    if (currentAccrualRecord.verified !=
        list_accruals[currentIndex].verified)
        return true;
    if (currentAccrualRecord.note != list_accruals[currentIndex].note)
        return true;

    return false;
}

double StatementsPanel::calculateSalaryBase(int statementId) const {
    double salaryBase = 0.0f;
    for (const auto &accrual : list_accruals) {
        if (accrual.statement_id == statementId && accrual.this_salary) {
            salaryBase += accrual.amount;
        }
    }
    return salaryBase;
}

void StatementsPanel::render() {
    if (!isOpen)
        return;
    // проверка на существование таблицы - вдруг база пауста или не та
    if (!db.tableExists("Statements")) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1),
                           "Табилца ведомостей отсуствует!");
        return;
    }

    //    ImGui::Begin(name.c_str(), &isOpen);
    bool goBottom = false;
    bool focusFirst = false;

    bool goAccrualBottom = false;
    bool focusAccrualFirst = false;

    const char *months[] = {"Январь",   "Февраль", "Март",   "Апрель",
                            "Май",      "Июнь",    "Июль",   "Август",
                            "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"};

    // Разделение окна на 3 части (1 фиксированная + 2 изменяемых)
    static float fixed_width = 600.0f;
    static float fixed_heigh = 400.0f;
    static float splitter_size = 5.0f;

    // Верхняя панель
    // ImGui::BeginChild("TopPanel", ImVec2(0, 0), true);
    // {
    float right_width = ImGui::GetContentRegionAvail().x;

    // Верхнюю делим на две части, фикс и  изменяемую
    ImGui::BeginChild("UpPanel", ImVec2(right_width, fixed_heigh), true);
    {
        // ImGui::Text("Изменяемая часть 1");
        // -----------------------------------------------------------------------------------------------------------
        ImGui::BeginGroup();
        // ImGui::Begin("Toolbar");
        // {

        // обновить
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.2f, 0.7f, 0.9f, 0.6f)); // голубой
        if (ImGui::Button(ICON_FA_REFRESH)) {

            writeToDatabase();
            writeAccrualToDatabase();
            refresh();
            // дергаем индекс, что бы система перечитала выделенное
            oldIndex = -1;
            // selectedIndex = -1;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Обновить данные");
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        // добавление записи
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.2f, 0.7f, 0.2f, 0.6f)); // Зеленый
        if (ImGui::Button(ICON_FA_PLUS)) {
            addRecord();
            refresh();
            // прыгвем на последнюю запись
            selectedIndex = statements.size() - 1;
            goBottom = true;
            focusFirst = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Добавить новую запись");
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        // удаление
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.9f, 0.1f, 0.1f, 0.6f)); // красный
        if (ImGui::Button(ICON_FA_TRASH)) {                    /* ... */
            if (selectedIndex >= 0)
                ImGui::OpenPopup("Удаление");
        }

        ImGui::PopStyleColor();
        // обработка удаления
        if (ImGui::BeginPopupModal("Удаление", NULL,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1, 1, 1, 0.6f), "Внимание!");
            ImGui::Text("Удаление выбранной записи.");
            ImGui::Separator();

            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.8f, 0.1f, 0.1f, 6.0f));
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
            ImGui::TextColored(ImVec4(1, 0, 0, 0.9f), "Внимание!");
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1, 0, 0, 0.6f),
                               "Удаление выбранной записи!");
            // ImGui::BulletText("Удаление выбранного сотрудника");
            ImGui::EndTooltip();
        }

        // Глобальный фильтр
        ImGui::SameLine();
        ImGui::Text("Фильтр:");
        ImGui::SameLine();
        // global_filter.Draw("##global_filter",
        //                    ImGui::GetContentRegionAvail().x - 100);
        global_filter.Draw("##global_filter",
                           (ImGui::GetContentRegionAvail().x - 100) * 0.5f);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Фильтр ведомостей");
        }
        ImGui::SameLine();
        global_filter_accurals.Draw("##global_filter_accurals",
                                    ImGui::GetContentRegionAvail().x - 100);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Фильтр начислений");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ERASER)) {
            global_filter.Clear();
            global_filter_accurals.Clear();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Очистить фильтр");
        }

        // Отчет по запросу
        ImGui::SameLine();
        // добавление записи
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.2f, 0.7f, 0.5f, 0.6f)); //
        if (ImGui::Button(ICON_FA_LIST)) {

            // Добавляем пнель запроса
            auto newPanel = std::make_unique<ReviewPanel>(
                db,
                "SELECT s.id, s.month, i.full_name, p.job_title, "
                "d.division_name, e.contract, printf('%.2f', p.salary), "
                "e.rate, s.hours_norm, s.hours_worked, s.timesheet_verified, "
                "s.note FROM Statements s LEFT JOIN Employees e ON "
                "s.employee_id=e.id LEFT JOIN Individuals i ON "
                "e.individual_id= i.id LEFT JOIN Positions p ON e.position_id "
                "= p.id LEFT JOIN Divisions d ON e.division_id = d.id"

            );
            // auto newPanel = std::make_unique<PositionsPanel>(db);
            manager_panels.addPanel(std::move(newPanel));
            manager_panels.getNextEnd() = true;
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Список ведомосей");
        }
        ImGui::PopStyleColor();

        // ImGui::End();

        ImGui::EndGroup();
        // -----------------------------------------------------------------------------------------------------------
        // Левая фиксированная часть - редактирование данных ведомости
        const double salaryBase = calculateSalaryBase(currentRecord.id);
        sSalary = salaryBase;

        ImGui::BeginChild("LeftPanel", ImVec2(fixed_width, 0), true);
        // ImGui::Text("Фиксированная часть (200px)");
        // -----------------------------------------------------------------------------------------------------------

        // поля редактирования
        // ImGui::SeparatorText("редактор");

        // ImGui::Separator();

        ImGui::Text("%s %d", "ID :", currentRecord.id);
        // если нужно, то вокус на поле ввода
        if (focusFirst) {
            ImGui::SetKeyboardFocusHere();
            focusFirst = false;
        }

        // месяц
        ImGui::Text("Месяц:");
        ImGui::SameLine();
        // ImGui::InputInt("##месяц_", &currentRecord.month);

        int current_month = currentRecord.month - 1; // 1 = Январь, 12 = Декабрь

        if (ImGui::Combo("##Месяц", &current_month, months,
                         IM_ARRAYSIZE(months))) {
            currentRecord.month = current_month + 1;
        }

        ImGui::Text("Сотрудник:");
        ImGui::SameLine();
        // открыть панель физлиц
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.2f, 0.5f, 0.2f, 0.6f)); // Зеленый
        if (ImGui::Button(ICON_FA_GROUP)) {
            auto newPanel = std::make_unique<EmployeesPanel>(db);
            manager_panels.addPanel(std::move(newPanel));
            manager_panels.getNextEnd() = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Сотрудники");
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        // комбобокс с фильтром
        if (ComboWithFilter("##СОТРУДНИКИ", currentRecord.employee_id,
                            employees)) {
            // Обработка изменения выбора
            auto it = std::find_if(employees.begin(), employees.end(),
                                   [&](const ComboItem &e) {
                                       return e.id == currentRecord.employee_id;
                                   });

            if (it != employees.end()) {
                // ImGui::Text("Выбрано: %s (ID: %d)", it3->name.c_str(),
                currentRecord.employee_id = it->id;
            }
        }

        ImGui::Text("Расчетные ( оклад: ");
        ImGui::SameLine();
        // расчет полного оклада по выплате для стравнения
        double rSalary = 0.0f;
        if (currentRecord.rate != 0 && currentRecord.hours_worked != 0)
            rSalary = salaryBase * currentRecord.hours_norm /
                      (currentRecord.rate * currentRecord.hours_worked);

        if (rSalary != currentRecord.salary)
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 0.6f), "%0.2f",
                               rSalary);
        else
            ImGui::Text("%0.2f", rSalary);
        ImGui::SameLine();
        ImGui::Text(" / %0.2f", currentRecord.salary);

        // ImGui::SetCursorPosX(
        //     ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
        //     ImGui::CalcTextSize(std::to_string(currentRecord.rate).c_str()).x);
        // ImGui::SetNextItemWidth(
        //     ImGui::CalcTextSize(std::to_string(currentRecord.rate).c_str()).x);
        // ImGui::InputDouble("##ставки", &currentRecord.rate, 0, 0, "%0.2f");

        ImGui::SameLine();
        ImGui::Text(" ставок:%0.2f", currentRecord.rate);
        // ImGui::SameLine();
        // ImGui::Text(" норма:%0.2f )",currentRecord.norm);

        ImGui::Text("Норма часов:");
        ImGui::SameLine();
        // ImGui::InputDouble("##норма", &currentRecord.hours_norm, 0, 0,
        // "%0.2f");
        if (InputDoubleWithCalculation("##норма", &currentRecord.hours_norm,
                                       "%0.2f")) {
            // Значение изменилось
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(" = +-*/ ");
        }
        ImGui::Text("Отработано часов:");
        ImGui::SameLine();
        // ImGui::InputDouble("##часы", &currentRecord.hours_worked, 0, 0,
        //                    "%0.2f");
        InputDoubleWithCalculation("##часы", &currentRecord.hours_worked,
                                   "%0.2f");

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(" = +-*/ ");
        }

        ToggleButton("Табель проверен:", currentRecord.timesheet_verified);

        ImGui::Text("Примечание:");
        // текст с автопереносом
        InputTextWrapper("##note", currentRecord.note,
                         ImGui::GetContentRegionAvail().x);

        // Таблица со списком

        // ImGui::SameLine();
        // ImGui::SeparatorText("справочник");

        // ImGui::PushStyleColor(ImGuiCol_Separator,
        //                       ImVec4(1, 0, 0, 1)); // Красный цвет
        // ImGui::Separator();
        // ImGui::PopStyleColor();

        // -----------------------------------------------------------------------------------------------------------
        ImGui::EndChild();

        ImGui::SameLine();

        // Сплиттер (право лево разделитель)
        ImGui::Button("##Splitter", ImVec2(splitter_size, -1));
        if (ImGui::IsItemActive()) {
            fixed_width += ImGui::GetIO().MouseDelta.x;
            fixed_width =
                std::clamp(fixed_width, 100.0f, 900.0f); // Ограничиваем размер
        }
        ImGui::SameLine();

        ImGui::BeginChild("RightPanel",
                          ImVec2(ImGui::GetContentRegionAvail().x, 0), true);
        // -----------------------------------------------------------------------------------------------------------
        // ImGui::Text("Изменяемая часть 2");

        // добавление записи
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.2f, 0.7f, 0.2f, 0.6f)); // Зеленый
        if (ImGui::Button(ICON_FA_PLUS)) {
            addAccrualRecord();
            refreshAccruals();
            // прыгвем на последнюю запись
            selectedAccrualIndex = list_accruals.size() - 1;
            goAccrualBottom = true;
            focusAccrualFirst = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Добавить начисление");
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        // удаление
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.9f, 0.1f, 0.1f, 0.6f)); // красный
        if (ImGui::Button(ICON_FA_TRASH)) {                    /* ... */
            if (selectedAccrualIndex >= 0)
                ImGui::OpenPopup("УдалениеНачисления");
        }

        ImGui::PopStyleColor();
        // обработка удаления
        if (ImGui::BeginPopupModal("УдалениеНачисления", NULL,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Внимание!");
            ImGui::Text("Удаление выбранного начисления.");
            ImGui::Separator();

            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.8f, 0.1f, 0.1f, 0.6f));
            if (ImGui::Button("Продолжить", ImVec2(120, 40))) {

                // Выполнняем удаление
                ImGui::CloseCurrentPopup();
                delAccrualRecord();
                refreshAccruals();
                // дергаем индекс, что бы система перечитала выделенное
                oldAccrualIndex = -1;
                selectedAccrualIndex--;
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
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Внимание!");
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1, 0, 0, 0.6f),
                               "Удаление выбранного начисления!");
            // ImGui::BulletText("Удаление выбранного сотрудника");
            ImGui::EndTooltip();
        }

        // открыть панель начислений
        ImGui::SameLine(0.0f, 40.0f);
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.2f, 0.5f, 0.2f, 0.6f)); // Зеленый
        if (ImGui::Button(ICON_FA_MONEY)) {
            auto newPanel = std::make_unique<AccrualsPanel>(db);
            manager_panels.addPanel(std::move(newPanel));
            manager_panels.getNextEnd() = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Начисления");
        }
        ImGui::PopStyleColor();

        // открыть панель приказов
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.2f, 0.5f, 0.2f, 0.6f)); // Зеленый
        if (ImGui::Button(ICON_FA_ORDER)) {
            auto newPanel = std::make_unique<OrdersPanel>(db);
            manager_panels.addPanel(std::move(newPanel));
            manager_panels.getNextEnd() = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Приказы");
        }
        ImGui::PopStyleColor();

        // сумма начислений

        ImGui::SameLine();
        ImGui::Text("Итого: %5.2f", summaAccirals);

        // таблица начислений
        // ----------------------------------------------------------------------------------------------
        if (ImGui::BeginTable(
                "list_accruals", 7,
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                    ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                    ImGuiTableFlags_ScrollX)) {

            ImGui::TableSetupColumn("ID",
                                    ImGuiTableColumnFlags_WidthFixed |
                                        ImGuiTableColumnFlags_NoResize,
                                    50.0f);
            ImGui::TableSetupColumn("Начисление");
            ImGui::TableSetupColumn("Сумма",
                                    ImGuiTableColumnFlags_WidthFixed |
                                        ImGuiTableColumnFlags_NoResize,
                                    150.0f);
            ImGui::TableSetupColumn("Процент",
                                    ImGuiTableColumnFlags_WidthFixed |
                                        ImGuiTableColumnFlags_NoResize,
                                    50.0f);
            ImGui::TableSetupColumn("Приказ");
            ImGui::TableSetupColumn("Проверено",
                                    ImGuiTableColumnFlags_WidthFixed |
                                        ImGuiTableColumnFlags_NoResize,
                                    30.0f);
            ImGui::TableSetupColumn("Примечание");
            ImGui::TableHeadersRow();

            summaAccirals = 0.00f;

            for (size_t i = 0; i < list_accruals.size(); ++i) {

                // фильр по номеру ведомости
                if (list_accruals[i].statement_id != currentRecord.id) {
                    continue;
                }

                // фильр по начислению
                std::string row_text_accurals;
                // if (global_filter_accurals.IsActive()) {
                row_text_accurals += list_accruals[i].accrual + " ";
                row_text_accurals += list_accruals[i].order + " ";
                // }
                // если не совпадает с фильтром, то пропускаем строку
                if (!global_filter_accurals.PassFilter(
                        row_text_accurals.c_str())) {
                    continue;
                }

                // расчет суммы начислений
                summaAccirals = summaAccirals + list_accruals[i].amount;

                // таблица начислений
                // --------------------------------------------------------
                // ЩЩЩЩЩЩЩЩo
                ImGui::TableNextRow();
                // Условия для цветов
                ImU32 row_color;
                // смотрим есть ли непроверенные начисления
                if (!list_accruals[i].verified &&
                    list_accruals[i].verification) {

                    row_color = ImGui::GetColorU32(
                        ImVec4(0.5f, 0.2f, 0.2f, 0.3f)); // Красный
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                                           row_color);
                }

                const bool isAccrualSelected =
                    currentAccrualRecord.id == list_accruals[i].id;

                // подсветка выбранной строки
                if (isAccrualSelected) {
                    ImGui::TableSetBgColor(
                        ImGuiTableBgTarget_RowBg0,
                        ImGui::GetColorU32(ImGuiCol_HeaderHovered));
                }
                // ID
                ImGui::TableSetColumnIndex(0);
                // выделение всей    строки
                if (isAccrualSelected) {
                    // если строка уже выделена, то просто текст ID
                    ImGui::Text("%s",
                                std::to_string(list_accruals[i].id).c_str());
                } else {
                    if (ImGui::Selectable(
                            std::to_string(list_accruals[i].id).c_str(),
                            isAccrualSelected,
                            ImGuiSelectableFlags_SpanAllColumns)) {
                        writeAccrualToDatabase();
                        selectedAccrualIndex = i;
                        oldAccrualIndex = i;
                        currentAccrualRecord = list_accruals[i];
                    }
                }

                // начисление
                ImGui::TableSetColumnIndex(1);
                if (isAccrualSelected) {
                    // комбобокс с фильтром
                    if (focusAccrualFirst) {
                        ImGui::SetKeyboardFocusHere(); // если добавлена новая
                                                       // запись, то смещаем
                                                       // фокус
                        focusAccrualFirst = false;
                        std::cout << "set focus " << std::endl;
                    }
                    if (ComboWithFilter("##ACC",
                                        currentAccrualRecord.accrual_id,
                                        accruals)) {
                        // Обработка изменения выбора
                        auto it = std::find_if(
                            accruals.begin(), accruals.end(),
                            [&](const ComboItem &e) {
                                return e.id == currentAccrualRecord.accrual_id;
                            });

                        if (it != accruals.end()) {
                            currentAccrualRecord.accrual_id = it->id;
                        }
                    }

                } else
                    ImGui::Text("%s", list_accruals[i].accrual.c_str());
                // сумма
                ImGui::TableSetColumnIndex(2);
                // прижимае к правой тороне
                if (isAccrualSelected) {
                    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
                    // ImGui::InputDouble("##сумма",
                    // &currentAccrualRecord.amount,
                    //                    0, 0, "%.2f");
                    InputDoubleWithCalculation(
                        "##amount", &currentAccrualRecord.amount, "%0.2f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(" = +-*/ ");
                    }
                } else {

                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                                        ImVec2(0, 0));
                    char buf_amount[32];
                    snprintf(buf_amount, sizeof(buf_amount), "%.2f",
                             list_accruals[i].amount);
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                                         ImGui::GetContentRegionAvail().x -
                                         ImGui::CalcTextSize(buf_amount).x);

                    ImGui::Text("%.2f", list_accruals[i].amount);

                    ImGui::PopStyleVar();
                }

                // Процент ----------------------------
                // расчет процента от оклада
                double rPercent = 0.0f;
                if (salaryBase != 0)
                    rPercent = list_accruals[i].amount / salaryBase * 100;
                // если процент не равен справочному, то красным
                ImGui::TableSetColumnIndex(3);
                if (rPercent != list_accruals[i].percentage)
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 0.8f), "%2.0f",
                                       rPercent);
                else
                    ImGui::Text("%2.0f", rPercent);

                // Приказ
                ImGui::TableSetColumnIndex(4);
                if (isAccrualSelected) {
                    // комбобокс с фильтром
                    if (ComboWithFilter("##ORD", currentAccrualRecord.order_id,
                                        orders)) {
                        // Обработка изменения выбора
                        auto it = std::find_if(
                            orders.begin(), orders.end(),
                            [&](const ComboItem &e) {
                                return e.id == currentAccrualRecord.order_id;
                            });

                        if (it != orders.end()) {
                            currentAccrualRecord.order_id = it->id;
                        }
                    }

                } else
                    ImGui::Text("%s", list_accruals[i].order.c_str());

                // Проверено
                ImGui::TableSetColumnIndex(5);
                if (isAccrualSelected) {
                    ImGui::Checkbox("##Verified",
                                    &currentAccrualRecord.verified);
                } else
                    ImGui::Text("%s", list_accruals[i].verified ? "+" : " ");

                // Примечание
                ImGui::TableSetColumnIndex(6);
                if (isAccrualSelected) {
                    ImGui::InputText("##note", &currentAccrualRecord.note);
                } else
                    ImGui::Text("%s", list_accruals[i].note.c_str());

                // ImGui::Text("%s", list_accruals[i].order.c_st /r());
                // примечание
                // ImGui::TableSetColumnIndex(7);
                // обрабатываем многострочку - просто срезаем после возврата
                // строки ImGui::Text("%s", list_accruals[i]
                //                       .note.substr(0,
                //                       list_accruals[i].note.find("\n"))
                //                       .c_str());
                // Прокручиваем к последнему элементу если выделена последняя
                // строка
                // - для добавленной записи
                if (goAccrualBottom && i == list_accruals.size() - 1) {
                    ImGui::SetScrollHereY(1.0f); // 1.0f = нижний край экрана
                    goAccrualBottom = false;
                    // focusAccrualFirst = true; // после прогрутки установить
                    // фокус
                }
            }
            ImGui::EndTable();
        }

        // -----------------------------------------------------------------------------------------------------------
        ImGui::EndChild();
    }
    ImGui::EndChild();

    // Сплиттер (верх низ разделитель)
    ImGui::Button("##Splitter2", ImVec2(-1, splitter_size));
    if (ImGui::IsItemActive()) {
        fixed_heigh += ImGui::GetIO().MouseDelta.y;
        fixed_heigh =
            std::clamp(fixed_heigh, 100.0f, 600.0f); // Ограничиваем размер
    }

    // Нижняя изменяемая часть (таблица расчетных ведомостей)
    ImGui::BeginChild("Bottom", ImVec2(right_width, 0), true);
    // -----------------------------------------------------------------------------------------------------------
    // ImGui::Text("Изменяемая часть 3");

    if (ImGui::BeginTable("Statements", 9,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_Sortable |
                              ImGuiTableFlags_SortMulti)) {

        ImGui::TableSetupColumn("ID",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                50.0f, 0);
        ImGui::TableSetupColumn("Месяц",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                100.0f, 1);
        ImGui::TableSetupColumn("Сотрудник", 0, 0.0f, 2);
        ImGui::TableSetupColumn("Оклад",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                150.0f, 3);
        ImGui::TableSetupColumn("Ставка",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                100.0f, 4);
        ImGui::TableSetupColumn("Норма",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                100.0f, 5);
        ImGui::TableSetupColumn("Часы",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                100.0f, 6);
        ImGui::TableSetupColumn("табель+",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                10.0f, 7);
        ImGui::TableSetupColumn("Примечание", 0, 0.0f, 8);
        ImGui::TableHeadersRow();

        sortStatements(statements, ImGui::TableGetSortSpecs());

        for (size_t i = 0; i < statements.size(); ++i) {

            // фильр - определяем нудна ли нам текущая строка для отображения

            // Собираем всю строку в один текст для фильтрации
            // фильтр медомостей
            std::string row_text;
            if (statements[i].month > 0)
                row_text += std::string(months[statements[i].month - 1]) + " ";
            row_text += statements[i].employee + " ";
            row_text += std::to_string(statements[i].salary) + " ";
            row_text += statements[i].note;
            // if (global_filter.IsActive())
            //     std::cout << row_text << std::endl;

            // если не совпадает с фильтром, то пропускаем строку
            if (!global_filter.PassFilter(row_text.c_str())) {
                continue;
            }
            // фильтр начтислений
            std::string row_text_accurals;
            if (global_filter_accurals.IsActive()) {
                for (size_t a = 0; a < list_accruals.size(); a++) {
                    if (list_accruals[a].statement_id == statements[i].id) {
                        row_text_accurals += list_accruals[a].accrual + " ";
                        row_text_accurals += list_accruals[a].order + " ";
                    }
                }
            }

            // если не совпадает с фильтром, то пропускаем строку
            if (!global_filter_accurals.PassFilter(row_text_accurals.c_str())) {
                continue;
            }
            // фильтр только по сотруднику
            // if (!global_filter.PassFilter(statements[i].employee.c_str())) {
            //      continue;
            // }

            // таблица
            ImGui::TableNextRow();

            // Условия для цветов
            ImU32 row_color;
            // если проверен табель
            if (statements[i].timesheet_verified) {
                row_color = ImGui::GetColorU32(
                    ImVec4(0.2f, 0.5f, 0.2f, 0.3f)); // зеленый
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, row_color);
            }

            // смотрим есть ли непроверенные начисления
            bool vrf = false;
            for (size_t a = 0; a < list_accruals.size(); a++) {
                if (list_accruals[a].statement_id == statements[i].id) {
                    vrf = vrf || (!list_accruals[a].verified &&
                                  list_accruals[a].verification);
                }
            }

            if (vrf) {
                row_color = ImGui::GetColorU32(
                    ImVec4(0.5f, 0.2f, 0.2f, 0.3f)); // Красный
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, row_color);
            }
            // ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, row_color);

            const bool isStatementSelected = currentRecord.id == statements[i].id;

            // ID
            ImGui::TableSetColumnIndex(0);
            // выделение всей    строки
            if (ImGui::Selectable(std::to_string(statements[i].id).c_str(),
                                  isStatementSelected,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                writeToDatabase();
                selectedIndex = i;
                oldIndex = i;
                currentRecord = statements[i];

                // сбрасываем выбранное начисление при смене ведомости
                selectedAccrualIndex = -1;
                oldAccrualIndex = -1;
                currentAccrualRecord = List_accrual{};
            }

            // месяц
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", statements[i].month > 0
                                  ? months[statements[i].month - 1]
                                  : "");
            // сотрудник
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", statements[i].employee.c_str());
            // оклад
            ImGui::TableSetColumnIndex(3);
            // прижимае к правой тороне
            // ImGui::Text("%30.2f", statements[i].salary);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            char buf_salary[32];
            snprintf(buf_salary, sizeof(buf_salary), "%0.2f",
                     statements[i].salary);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                                 ImGui::GetContentRegionAvail().x -
                                 ImGui::CalcTextSize(buf_salary).x);
            ImGui::Text("%0.2f", statements[i].salary);
            ImGui::PopStyleVar();
            // ставка
            ImGui::TableSetColumnIndex(4);
            // прижимае к правой тороне
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            char buf_rate[32];
            snprintf(buf_rate, sizeof(buf_rate), "%.2f", statements[i].rate);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                                 ImGui::GetContentRegionAvail().x -
                                 ImGui::CalcTextSize(buf_rate).x);
            ImGui::Text("%0.2f", statements[i].rate);
            ImGui::PopStyleVar();
            // норма факт
            ImGui::TableSetColumnIndex(5);
            // прижимае к правой стороне
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            char buf_nhours[32];
            snprintf(buf_nhours, sizeof(buf_nhours), "%.2f",
                     statements[i].hours_norm);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                                 ImGui::GetContentRegionAvail().x -
                                 ImGui::CalcTextSize(buf_nhours).x);
            ImGui::Text("%0.2f", statements[i].hours_norm);
            ImGui::PopStyleVar();
            // часы
            ImGui::TableSetColumnIndex(6);
            // прижимае к правой тороне
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            char buf_hours[32];
            snprintf(buf_hours, sizeof(buf_hours), "%.2f",
                     statements[i].hours_worked);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                                 ImGui::GetContentRegionAvail().x -
                                 ImGui::CalcTextSize(buf_hours).x);
            ImGui::Text("%0.2f", statements[i].hours_worked);
            ImGui::PopStyleVar();

            // табель сверен
            ImGui::TableSetColumnIndex(7);
            ImGui::Text("%s", statements[i].timesheet_verified ? "+" : " ");
            // примечание
            ImGui::TableSetColumnIndex(8);
            // обрабатываем многострочку - просто срезаем после возврата строки
            ImGui::Text("%s", statements[i]
                                  .note.substr(0, statements[i].note.find("\n"))
                                  .c_str());
            // Прокручиваем к последнему элементу если выделена последняя строка
            // - для добавленной записи
            if (goBottom && i == statements.size() - 1) {
                ImGui::SetScrollHereY(1.0f); // 1.0f = нижний край экрана
                goBottom = false;
            }
        }
        ImGui::EndTable();
    }

    // -----------------------------------------------------------------------------------------------------------
    ImGui::EndChild();
    // }
    // ImGui::EndChild();
    // ImGui::BeginChild(name.c_str(),ImVec2(fixed_width, 0), true);
    // ImGui::BeginChild(name.c_str());

    //    ImGui::End();

    // ImGui::EndChild();

    // ImGui::BeginChild();
    // setStatement(listPanel.GetSelStatement());

    //    ImGui::Begin(name.c_str(), &isOpen);

    // ImGui::BeginChild(name.c_str());

    //    ImGui::End();

    // std::cout << ". ";
    // ImGui::EndChild();
}
