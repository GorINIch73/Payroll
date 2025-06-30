#include "imgui_components.h"
#include <algorithm>
#include <imgui.h>
#include <string>
#include <vector>

#include <unicode/utf8.h>

// реализация кастомного чекбокса
bool ToggleButton(const char *label, bool *v) {

    ImGui::TextUnformatted(label);
    ImGui::SameLine();

    ImGuiStyle &style = ImGui::GetStyle();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.8f;
    float radius = height * 0.50f;

    if (ImGui::InvisibleButton(label, ImVec2(width, height))) {
        *v = !*v;
    }

    float t = *v ? 1.0f : 0.0f;
    ImU32 col_bg = ImGui::GetColorU32(*v ? ImVec4(0.23f, 0.73f, 0.23f, 1.0f)
                                         : ImVec4(0.73f, 0.23f, 0.23f, 1.0f));

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg,
                             height * 0.5f);
    draw_list->AddCircleFilled(
        ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius),
        radius - 1.5f, IM_COL32(255, 255, 255, 255));

    return *v;
}

// реализация текстового пля ввода с посимвольным автопереносом
bool InputTextWrapper(const char *label, std::string &text, float width) {

    bool changed = false;

    // Добавляем переносы вручную
    std::string result = "";
    // float width = ImGui::GetContentRegionAvail().x;
    float current_line_width = 0.0f;

    int i = 0;

    while (i < text.size()) {
        int old_i = i;
        UChar32 c;
        U8_NEXT(text.c_str(), i, text.size(), c);

        // посчитать размер
        std::string char_s = text.substr(old_i, i - old_i);
        float char_width = ImGui::CalcTextSize(char_s.c_str()).x;

        if (current_line_width + char_width > width && text[i] != '\n') {
            result.push_back('\n');
            current_line_width = 0;
        }
        result.append(char_s);
        current_line_width += char_width;
    }

    std::vector<char> buffer(result.begin(), result.end());
    buffer.push_back('\0');
    buffer.resize(text.size() + 1024);

    ImGui::PushID(label);
    ImGui::PushTextWrapPos(width);
    changed =
        ImGui::InputTextMultiline(label, buffer.data(), buffer.size(),
                                  ImVec2(width, ImGui::GetTextLineHeight() * 3),
                                  ImGuiInputTextFlags_NoHorizontalScroll);

    if (changed) {
        // Удаляем добавленные переносы перед сохранением
        text = buffer.data();
        text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
    }

    ImGui::PopTextWrapPos();

    ImGui::PopID();

    return changed;
}

// реализация комбобокса с фильром
//
bool ComboWithFilter(const char *label, int *current_id,
                     std::vector<ComboItem> &items, ImGuiComboFlags flags) {
    bool changed = false;
    static char search_buffer[128] = "";
    static bool just_opened = false;

    // Находим текущий элемент для превью
    std::string preview = "Не выбрано";
    auto current_it =
        std::find_if(items.begin(), items.end(),
                     [&](const ComboItem &i) { return i.id == *current_id; });
    if (current_it != items.end()) {
        preview = current_it->name;
    }

    ImGui::PushID(label);

    // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));
    // ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 5.0f);

    ImGui::PushItemWidth(-FLT_MIN);
    // Основной комбобокс
    if (ImGui::BeginCombo("##combo", preview.c_str(),
                          flags | ImGuiComboFlags_PopupAlignLeft)) {
        // Поле поиска
        ImGui::PushItemWidth(-FLT_MIN);
        // При первом открытии фокусируемся на поле ввода
        if (just_opened) {
            // не работает нормально - пока что без
            ImGui::SetKeyboardFocusHere();
            just_opened = false;
        }
        if (ImGui::InputText("##search", search_buffer,
                             IM_ARRAYSIZE(search_buffer),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            // При нажатии Enter выбираем первый подходящий вариант
            if (!items.empty()) {
                if (search_buffer[0] == '\0') {
                    // Если строка пустая - выбираем первый элемент
                    *current_id = items[0].id;
                } else {
                    // Ищем первый подходящий элемент
                    auto it = std::find_if(
                        items.begin(), items.end(), [&](const ComboItem &item) {
                            return item.name.find(search_buffer) !=
                                   std::string::npos;
                        });
                    if (it != items.end()) {
                        *current_id = it->id;
                    }
                }
                changed = true;
                ImGui::CloseCurrentPopup();
            }
            // При нажатии Enter выбираем первый подходящий вариант
            // if (!items.empty()) {
            //     if (search_buffer[0] != '\0') {
            //         // Ищем первый подходящий элемент
            //         auto it = std::find_if(
            //             items.begin(), items.end(), [&](const ComboItem
            //             &item) {
            //                 return item.name.find(search_buffer) !=
            //                        std::string::npos;
            //             });
            //         if (it != items.end()) {
            //             *current_id = it->id;
            //         }
            //     }
            //     changed = true;
            //     // ImGui::CloseCurrentPopup();
            // }
        }
        ImGui::PopItemWidth();

        // Фильтрация элементов
        std::vector<ComboItem *> filtered_items;
        for (auto &item : items) {
            if (search_buffer[0] == '\0' ||
                item.name.find(search_buffer) != std::string::npos) {
                filtered_items.push_back(&item);
            }
        }

        // Отображение отфильтрованных элементов
        if (ImGui::BeginListBox(
                "##list",
                ImVec2(-FLT_MIN,
                       std::min(filtered_items.size() *
                                        ImGui::GetTextLineHeightWithSpacing() +
                                    30,
                                200.0f)))) {
            for (auto item : filtered_items) {
                bool is_selected = (*current_id == item->id);
                // Пропускаем или обрабатываем пустые элементы
                if (ImGui::Selectable(
                        (!item->name.empty() ? item->name.c_str() : "-ПУСТО-"),
                        is_selected)) {
                    *current_id = item->id;
                    changed = true;
                    search_buffer[0] = '\0'; // Сброс поиска
                    ImGui::CloseCurrentPopup();
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
        ImGui::EndCombo();
    } else {
        // не работает
        if (changed)
            just_opened = true; // Запоминаем что комбобокс только что закрылся
    }
    // Автоподстановка при вводе
    if (ImGui::IsItemActive() && search_buffer[0] != '\0') {
        auto exact_match =
            std::find_if(items.begin(), items.end(), [&](const ComboItem &i) {
                return i.name == search_buffer;
            });

        if (exact_match != items.end()) {
            *current_id = exact_match->id;
            changed = true;
            search_buffer[0] = '\0';
        }
    }

    // ImGui::PopStyleVar(2);

    ImGui::PopID();
    // ImGui::SameLine(0, 5);
    // ImGui::TextUnformatted(label);

    return changed;
}
