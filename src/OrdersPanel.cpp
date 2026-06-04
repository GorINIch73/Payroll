
#include "OrdersPanel.h"
#include "Icons.h"
#include "Manager.h"
#include "ReviewPanel.h"
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

namespace {
const char *monthName(int month) {
    static const char *months[] = {
        "Январь",   "Февраль", "Март",   "Апрель",   "Май",    "Июнь",
        "Июль",     "Август",  "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"};

    if (month < 1 || month > 12)
        return "";

    return months[month - 1];
}

int compareOrderAccrualDetails(const OrderAccrualDetail &left,
                               const OrderAccrualDetail &right,
                               ImGuiID columnId) {
    switch (columnId) {
    case 0:
        return left.full_name.compare(right.full_name);
    case 1:
        return left.job_title.compare(right.job_title);
    case 2:
        return left.month - right.month;
    case 3:
        return left.accrual.compare(right.accrual);
    case 4:
        return (left.percent > right.percent) - (left.percent < right.percent);
    case 5:
        return static_cast<int>(left.verified) - static_cast<int>(right.verified);
    case 6:
        return left.note.compare(right.note);
    default:
        return 0;
    }
}

void sortOrderAccrualDetails(std::vector<OrderAccrualDetail> &details,
                             const ImGuiTableSortSpecs *sortSpecs) {
    if (!sortSpecs || sortSpecs->SpecsCount == 0)
        return;

    std::stable_sort(
        details.begin(), details.end(),
        [sortSpecs](const OrderAccrualDetail &left,
                    const OrderAccrualDetail &right) {
            for (int i = 0; i < sortSpecs->SpecsCount; ++i) {
                const ImGuiTableColumnSortSpecs &spec = sortSpecs->Specs[i];
                int result = compareOrderAccrualDetails(left, right,
                                                        spec.ColumnUserID);
                if (result == 0)
                    continue;

                return spec.SortDirection == ImGuiSortDirection_Ascending
                           ? result < 0
                           : result > 0;
            }

            return left.list_accrual_id < right.list_accrual_id;
        });
}
} // namespace

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
              ", protocol_found=" +
              std::to_string(currentRecord.protocol_found ? 1 : 0) +
              ", note='" + currentRecord.note +
              "' WHERE id=" + std::to_string(currentRecord.id) + ";";

        // std::cout << currentRecord.note << std::endl;
        // std::cout << sql << std::endl;

        printf("Обновление записи ... \n");
        const bool saved = db.Execute(sql);
        if (saved)
            UpdateItemById(orders, currentRecord);
        return saved;
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
        sql =
            "DELETE FROM Orders WHERE id =" + std::to_string(currentRecord.id) +
            ";";
        return db.Execute(sql);
    }

    return false;
}

void OrdersPanel::refresh() {
    orders.clear();
    // Загружаем данные из БД (упрощенный пример)
    const char *sql =
        "SELECT id, number, date, found, protocol_found, note FROM Orders;";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<Order> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                Order{std::stoi(argv[0]), argv[1] ? argv[1] : "",
                      argv[2] ? argv[2] : "2000-01-01",
                      argv[3] ? (std::stoi(argv[3]) > 0 ? true : false) : false,
                      argv[4] ? (std::stoi(argv[4]) > 0 ? true : false) : false,
                      argv[5] ? argv[5] : ""});
            return 0;
        },
        &orders, nullptr);
}

void OrdersPanel::refreshAccrualDetails() {
    accrualDetails.clear();
    if (currentRecord.id < 0)
        return;

    const char *sql = R"(
        SELECT
            l.id,
            i.full_name,
            p.job_title,
            s.month,
            a.name,
            CASE
                WHEN salary_base.amount IS NOT NULL AND salary_base.amount <> 0
                THEN l.amount / salary_base.amount * 100
                ELSE 0
            END AS percent,
            l.verified,
            l.note
        FROM List_accruals l
        LEFT JOIN Accruals a ON l.accrual_id = a.id
        LEFT JOIN Statements s ON s.id = l.statement_id
        LEFT JOIN Employees e ON e.id = s.employee_id
        LEFT JOIN Individuals i ON e.individual_id = i.id
        LEFT JOIN Positions p ON e.position_id = p.id
        LEFT JOIN (
            SELECT l2.statement_id, SUM(l2.amount) AS amount
            FROM List_accruals l2
            JOIN Accruals a2 ON a2.id = l2.accrual_id
            WHERE a2.this_salary = 1
            GROUP BY l2.statement_id
        ) salary_base ON salary_base.statement_id = l.statement_id
        WHERE l.order_id = ?
        ORDER BY i.full_name, s.month, a.name;
    )";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db.getHandle(), sql, -1, &stmt, nullptr) !=
        SQLITE_OK) {
        std::cerr << "Failed to prepare order details query: "
                  << sqlite3_errmsg(db.getHandle()) << std::endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, currentRecord.id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        accrualDetails.push_back(OrderAccrualDetail{
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))
                ? reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))
                : "",
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2))
                ? reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2))
                : "",
            sqlite3_column_int(stmt, 3),
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4))
                ? reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4))
                : "",
            sqlite3_column_double(stmt, 5),
            sqlite3_column_int(stmt, 6) > 0,
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7))
                ? reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7))
                : "",
        });
    }

    sqlite3_finalize(stmt);
}

bool OrdersPanel::isCurrentChanged() {

    if (currentRecord.id < 0)
        return false;

    int currentIndex = FindIndexById(orders, currentRecord.id);
    if (currentIndex < 0)
        return false;

    // std::cout << " тест " << str << std::endl;
    // std::cout << " новое  :" << currentRecord.note << std::endl;
    // срапвниваем поля
    if (currentRecord.number != orders[currentIndex].number)
        return true;
    if (currentRecord.date != orders[currentIndex].date)
        return true;
    if (currentRecord.found != orders[currentIndex].found)
        return true;
    if (currentRecord.protocol_found !=
        orders[currentIndex].protocol_found)
        return true;
    if (currentRecord.note != orders[currentIndex].note)
        return true;

    return false;
}

bool OrdersPanel::saveAccrualDetail(const OrderAccrualDetail &detail) {
    if (detail.list_accrual_id < 0)
        return false;

    return db.ExecutePrepared(
        "UPDATE List_accruals SET verified = ?, note = ? WHERE id = ?;",
        {detail.verified, detail.note, detail.list_accrual_id});
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
    static float ordersListHeight = 240.0f;
    static float splitterSize = 5.0f;

    ImGui::BeginChild(name.c_str());

    ImGui::BeginGroup();
    // ImGui::Begin("Toolbar");
    // {

    // обновить
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.2f, 0.7f, 0.9f, 0.6f)); // голубой
    if (ImGui::Button(ICON_FA_REFRESH)) {
        writeToDatabase();
        refresh();
        refreshAccrualDetails();
        // дергаем индекс, что бы система перечитала выделенное
        oldIndex = -1;
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
        writeToDatabase();
        if (addRecord()) {
            const int newId =
                static_cast<int>(sqlite3_last_insert_rowid(db.getHandle()));
            refresh();
            SelectItemById(orders, newId, currentRecord, selectedIndex);
            oldIndex = selectedIndex;
            refreshAccrualDetails();
            goBottom = true;
            focusFirst = true;
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Добавить новый приказ");
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
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Внимание!");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1, 0, 0, 0.6f),
                           "Удаление выбранного начисления!");
        // ImGui::BulletText("Удаление выбранного сотрудника");
        ImGui::EndTooltip();
    }

    // Отчет по запросу
    ImGui::SameLine();
    // добавление записи
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.5f, 0.6f)); //
    if (ImGui::Button(ICON_FA_LIST)) {

        // Добавляем пнель запроса
	        auto newPanel = std::make_unique<ReviewPanel>(
	            db,
	            "SELECT o.number || '-' || o.date AS 'Приказ', "
	            "a.name AS 'Начисление', "
	            "i.full_name || '-' || p.job_title || '-' || d.division_name || "
	            "'-' || e.rate || '-' || e.contract AS 'Сотрудник', "
	            "printf('%.2f', l.amount) AS 'Сумма', "
	            "printf('%.0f', "
	            "CASE WHEN salary_base.amount IS NOT NULL AND salary_base.amount <> 0 "
	            "THEN l.amount / salary_base.amount * 100 ELSE 0 END"
	            ") AS 'Процент от оклада', "
	            "printf('%.0f', a.percentage) AS 'Процент начисления', "
	            "l.verified AS 'Проверено', l.note AS 'Примечание' "
	            "FROM Orders o "
	            "LEFT JOIN List_accruals l ON l.order_id = o.id "
	            "LEFT JOIN Accruals a ON l.accrual_id = a.id "
	            "LEFT JOIN Statements s ON s.id = l.statement_id "
	            "LEFT JOIN Employees e ON e.id = s.employee_id "
	            "LEFT JOIN Individuals i ON e.individual_id = i.id "
	            "LEFT JOIN Positions p ON e.position_id = p.id "
	            "LEFT JOIN Divisions d ON e.division_id = d.id "
	            "LEFT JOIN ("
	            "    SELECT l2.statement_id, SUM(l2.amount) AS amount "
	            "    FROM List_accruals l2 "
	            "    JOIN Accruals a2 ON a2.id = l2.accrual_id "
	            "    WHERE a2.this_salary = 1 "
	            "    GROUP BY l2.statement_id"
	            ") salary_base ON salary_base.statement_id = l.statement_id "
	            "ORDER BY o.number, o.date, a.name, i.full_name");
        // auto newPanel = std::make_unique<PositionsPanel>(db);
        manager_panels.addPanel(std::move(newPanel));
        manager_panels.getNextEnd() = true;
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Приказы в разрезе начислений");
    }
    ImGui::PopStyleColor();
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

    float availableHeight = ImGui::GetContentRegionAvail().y;
    float maxOrdersListHeight = std::max(140.0f, availableHeight - 160.0f);
    ordersListHeight =
        std::clamp(ordersListHeight, 120.0f, maxOrdersListHeight);

    if (ImGui::BeginTable("Orders", 6,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY,
                          ImVec2(0, ordersListHeight))) {

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
            bool isSelected = currentRecord.id == orders[i].id;
            if (ImGui::Selectable(std::to_string(orders[i].id).c_str(),
                                  isSelected,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                const int selectedId = orders[i].id;
                writeToDatabase();
                SelectItemById(orders, selectedId, currentRecord,
                               selectedIndex);
                oldIndex = selectedIndex;
                refreshAccrualDetails();
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
            ImGui::Text(
                "%s",
                orders[i].note.substr(0, orders[i].note.find("\n")).c_str());
            // Прокручиваем к последнему элементу если выделена последняя строка
            // - для добавленной записи
            if (goBottom && i == orders.size() - 1) {
                ImGui::SetScrollHereY(1.0f); // 1.0f = нижний край экрана
                goBottom = false;
            }
        }
        ImGui::EndTable();
    }

    ImGui::Button("##OrdersDetailsSplitter", ImVec2(-1, splitterSize));
    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        ImGui::SetTooltip("Изменить высоту списка приказов");
    }
    if (ImGui::IsItemActive()) {
        ordersListHeight += ImGui::GetIO().MouseDelta.y;
        ordersListHeight =
            std::clamp(ordersListHeight, 120.0f, maxOrdersListHeight);
    }

    ImGui::SeparatorText("Расшифровка начислений по приказу");
    if (currentRecord.id < 0) {
        ImGui::TextDisabled("Выберите приказ.");
    } else if (ImGui::BeginTable(
                   "OrderAccrualDetails", 7,
                   ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                       ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                       ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti,
                   ImVec2(0, 0))) {
        ImGui::TableSetupColumn("ФИО", 0, 0.0f, 0);
        ImGui::TableSetupColumn("Должность", 0, 0.0f, 1);
        ImGui::TableSetupColumn("Месяц",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                100.0f, 2);
        ImGui::TableSetupColumn("Начисление", 0, 0.0f, 3);
        ImGui::TableSetupColumn("% от оклада",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                90.0f, 4);
        ImGui::TableSetupColumn("Сверено",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoResize,
                                70.0f, 5);
        ImGui::TableSetupColumn("Примечание", 0, 0.0f, 6);
        ImGui::TableHeadersRow();

        sortOrderAccrualDetails(accrualDetails, ImGui::TableGetSortSpecs());

        for (size_t i = 0; i < accrualDetails.size(); ++i) {
            auto &detail = accrualDetails[i];
            ImGui::PushID(detail.list_accrual_id);

            ImGui::TableNextRow();
            if (!detail.verified) {
                ImGui::TableSetBgColor(
                    ImGuiTableBgTarget_RowBg0,
                    ImGui::GetColorU32(ImVec4(0.5f, 0.2f, 0.2f, 0.3f)));
            }

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", detail.full_name.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", detail.job_title.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", monthName(detail.month));

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", detail.accrual.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%.0f", detail.percent);

            ImGui::TableSetColumnIndex(5);
            if (ImGui::Checkbox("##verified", &detail.verified)) {
                saveAccrualDetail(detail);
            }

            ImGui::TableSetColumnIndex(6);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputText("##note", &detail.note,
                                 ImGuiInputTextFlags_EnterReturnsTrue) ||
                ImGui::IsItemDeactivatedAfterEdit()) {
                saveAccrualDetail(detail);
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();
}
