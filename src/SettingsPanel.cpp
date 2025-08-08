
#include "SettingsPanel.h"
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

SettingsPanel::SettingsPanel(Database &db)
    : Panel("Настройки"),
      db(db) {
    refresh();
    // std::cout << "проехали конструктор " << std::endl;
}
SettingsPanel::~SettingsPanel() { writeToDatabase(); }

bool SettingsPanel::writeToDatabase() {
    // Сохранение записи из редактора с проверкой изменения
    if(!isOpen) return false; // если не открыто, ничего не делаем

    std::string sql;
    sql = "UPDATE Settings SET organization='" + settingsDB[0].organization +
              "', note='" + settingsDB[0].note +
    "';";

    printf("сохранение настроек в базу ... \n");
    return db.Execute(sql);
}

bool SettingsPanel::clearDatabase() {
    // очистка базы
    // пока заглушка!
    std::string sql;
    sql = "UPDATE Settings SET organization='" + settingsDB[0].organization +
              "', note='" + settingsDB[0].note +
              "' WHERE id= 1";

    printf("очистка базы ... \n");
    return db.Execute(sql);
}


void SettingsPanel::refresh() {
    settingsDB.clear();
    // Загружаем данные из БД нужна 1 строка 
    const char *sql = "SELECT id, organization, note FROM Settings LIMIT 1;";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<SettingDB> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(SettingDB{std::stoi(argv[0]), argv[1],
                                          argv[2] ? argv[2] : ""});
            return 0;
        },
        &settingsDB, nullptr);
}


void SettingsPanel::render() {
    if (!isOpen)
        return;
    // проверка на существование таблицы - вдруг база пауста или не та
    if (!db.tableExists("Settings")) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Табилца настроек отсуствует!");
        return;
    }


    ImGui::BeginChild(name.c_str());

    ImGui::BeginGroup();
    // ImGui::Begin("Toolbar");
    // {

    // обновить
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.2f, 0.7f, 0.9f, 1.0f)); // голубой
    if (ImGui::Button(ICON_FA_SAVE)) {
        writeToDatabase();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Сохранить изменения");
    }
    ImGui::PopStyleColor();

    // }
    // ImGui::End();

    ImGui::EndGroup();
    // ImGui::EndChild();
    // поля редактирования
    // ImGui::SeparatorText("редактор");

    ImGui::Separator();

    ImGui::Text("Организация:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(
        ImGui::GetColumnWidth()); // Растянуть на всю колонку
    ImGui::InputText("##ОРГ", &settingsDB[0].organization);
    // ImGui::InputText("Примечание", &currentRecord.note);

    // мультистрочник - морока прям
    ImGui::Text("Примечание:");
    // Поле с автопереносом
    InputTextWrapper("##kh", settingsDB[0].note,
                     ImGui::GetContentRegionAvail().x);
    //
    // Таблица со списком

    // ImGui::SameLine();
    // ImGui::SeparatorText("справочник");

    // ImGui::PushStyleColor(ImGuiCol_Separator,
    //                       ImVec4(1, 0, 0, 1)); // Красный цвет
    ImGui::Separator();
    // ImGui::PopStyleColor();


    // очистить
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.9f, 0.1f, 0.1f, 1.0f)); // красный
    if (ImGui::Button(ICON_FA_TRASH "Очистка выбранных таблиц")) {                    /* ... */

        // проверить что что то выбрано
        ImGui::OpenPopup("Очистка");
    }

    ImGui::PopStyleColor();
    // обработка удаления
    if (ImGui::BeginPopupModal("Очистка", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Внимание!");
        ImGui::Text("Очистка выбранных данных.");
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Продолжить", ImVec2(120, 40))) {

            // возможно надо сохранить все изменения до очистки - зх как это сделать


            // Выполнняем очистку -
            clearDatabase();
            ImGui::CloseCurrentPopup();
            refresh();
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
                           "Удаление данных для выбранных таблиц!");
        ImGui::EndTooltip();
    }




    ImGui::EndChild();
}
