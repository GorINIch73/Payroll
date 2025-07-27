#pragma once

#include <algorithm>
#include <imgui.h>
#include <string>
#include <vector>

struct ComboItem {
        int id;
        std::string name;
};

// комбобока с фильтом и подстановкой
bool ComboWithFilter(const char *label, int &current_id,
                     std::vector<ComboItem> &items, ImGuiComboFlags flags = 0);
// Текстовое поле ввода с посимвольным автопереносом
bool InputTextWrapper(const char *label, std::string &text, float width);

// кастомный чекбокс:w
bool ToggleButton(const char *label, bool &v);

// Функция для рендеринга поля с автоматическими переходами
bool InputDate(const char* label, std::string &date);
