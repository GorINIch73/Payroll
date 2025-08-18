#pragma once

#include <algorithm>
#include <filesystem>
#include <imgui.h>
#include <string>
#include <vector>

// поиск шрифтов
std::filesystem::path find_font(const std::string &fontName);

// отображение лога событий
struct MessageLog {
        std::vector<std::pair<std::string, ImVec4>> messages;
        bool auto_scroll = true;
        float height = 22.0f; // Высота панели сообщений
        // float width = 200.f; // ширина поля

        void Add(const std::string &msg, ImVec4 color);
        void Add(const std::string &msg);
        void Draw();
};

// Глобальный экземпляр лога
inline MessageLog g_MessageLog;

// модальное сообщение об ошибке
void ShowErrorModal(const std::string title, std::string message,
                    bool *p_open = NULL);

// вспомогательная структура для комбобокса
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
bool InputDate(const char *label, std::string &date);
