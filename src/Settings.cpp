#include "Settings.h"
#include "Icons.h"
#include "SettingsPanel.h"
#include "imgui_components.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <fstream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <sstream>

void Settings::LoadFonts(float size) {

    if (!needLoadFonts)
        return;

    needLoadFonts = false;

    if (size > 0)
        sizeFont = size;

    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();
    style.FontSizeBase = sizeFont;
    style._NextFrameFontSizeBase = sizeFont;

    // Меняем набор шрифтов между кадрами, оставляя динамический атлас текстур
    // под управлением ImGui/OpenGL backend.
    io.Fonts->ClearFonts();

    // грузим русский шрифт
    ImFont *font = io.Fonts->AddFontFromFileTTF(
        find_font("NotoSans-Regular.ttf").string().c_str(), sizeFont, nullptr,
        io.Fonts->GetGlyphRangesCyrillic());

    // Добавляем иконки
    ImFontConfig config;
    // Добавляем Font Awesome
    config.MergeMode = true;
    config.PixelSnapH = true;

    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    io.Fonts->AddFontFromFileTTF(
        find_font("fontawesome-webfont.ttf").string().c_str(), sizeFont,
        &config, icons_ranges);
    io.FontDefault = font;
    // Перестраиваем описание атласа. Текстура обновится backend-ом при рендере.
    io.Fonts->Build();
}

namespace {
std::string trim(const std::string &value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos)
        return "";

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

bool parseBool(const std::string &value, bool fallback) {
    const std::string normalized = trim(value);
    if (normalized == "1" || normalized == "true" || normalized == "yes")
        return true;
    if (normalized == "0" || normalized == "false" || normalized == "no")
        return false;
    return fallback;
}
} // namespace

int Settings::GetThemeCount() const { return 9; }

const char *Settings::GetThemeName(int themeIndex) const {
    static const char *names[] = {"Темная",       "Светлая", "Классическая",
                                  "Графит",      "Лесная",  "Сепия",
                                  "Контрастная", "Розовая", "Коричневая"};

    if (themeIndex < 0 || themeIndex >= GetThemeCount())
        return names[0];

    return names[themeIndex];
}

void Settings::ApplyTheme() {
    ImGuiStyle &style = ImGui::GetStyle();

    switch (theme) {
    case 1:
        ImGui::StyleColorsLight();
        break;
    case 2:
        ImGui::StyleColorsClassic();
        break;
    case 3: {
        ImGui::StyleColorsDark();
        auto &c = style.Colors;
        c[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.12f, 0.14f, 1.00f);
        c[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
        c[ImGuiCol_Header] = ImVec4(0.22f, 0.30f, 0.38f, 0.75f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.40f, 0.50f, 0.85f);
        c[ImGuiCol_Button] = ImVec4(0.20f, 0.28f, 0.36f, 0.80f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.38f, 0.48f, 0.90f);
        c[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.19f, 0.22f, 1.00f);
        c[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.75f, 1.00f, 1.00f);
        break;
    }
    case 4: {
        ImGui::StyleColorsDark();
        auto &c = style.Colors;
        c[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.12f, 0.10f, 1.00f);
        c[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.15f, 0.12f, 1.00f);
        c[ImGuiCol_Header] = ImVec4(0.18f, 0.36f, 0.24f, 0.75f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.48f, 0.32f, 0.90f);
        c[ImGuiCol_Button] = ImVec4(0.16f, 0.34f, 0.23f, 0.82f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.46f, 0.31f, 0.92f);
        c[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.20f, 0.15f, 1.00f);
        c[ImGuiCol_CheckMark] = ImVec4(0.60f, 0.90f, 0.55f, 1.00f);
        break;
    }
    case 5: {
        ImGui::StyleColorsLight();
        auto &c = style.Colors;
        c[ImGuiCol_WindowBg] = ImVec4(0.92f, 0.89f, 0.82f, 1.00f);
        c[ImGuiCol_ChildBg] = ImVec4(0.96f, 0.94f, 0.88f, 1.00f);
        c[ImGuiCol_Text] = ImVec4(0.18f, 0.14f, 0.10f, 1.00f);
        c[ImGuiCol_Header] = ImVec4(0.62f, 0.47f, 0.30f, 0.35f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.62f, 0.47f, 0.30f, 0.55f);
        c[ImGuiCol_Button] = ImVec4(0.64f, 0.48f, 0.32f, 0.55f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.64f, 0.48f, 0.32f, 0.75f);
        c[ImGuiCol_FrameBg] = ImVec4(0.99f, 0.97f, 0.91f, 1.00f);
        break;
    }
    case 6: {
        ImGui::StyleColorsDark();
        auto &c = style.Colors;
        c[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
        c[ImGuiCol_ChildBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        c[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        c[ImGuiCol_Header] = ImVec4(0.95f, 0.70f, 0.10f, 0.55f);
        c[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 0.82f, 0.20f, 0.70f);
        c[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        c[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        c[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.86f, 0.20f, 1.00f);
        break;
    }
    case 7: {
        ImGui::StyleColorsLight();
        auto &c = style.Colors;
        c[ImGuiCol_WindowBg] = ImVec4(0.99f, 0.93f, 0.96f, 1.00f);
        c[ImGuiCol_ChildBg] = ImVec4(1.00f, 0.96f, 0.98f, 1.00f);
        c[ImGuiCol_Text] = ImVec4(0.22f, 0.12f, 0.18f, 1.00f);
        c[ImGuiCol_Header] = ImVec4(0.82f, 0.22f, 0.48f, 0.32f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.82f, 0.22f, 0.48f, 0.52f);
        c[ImGuiCol_HeaderActive] = ImVec4(0.72f, 0.16f, 0.40f, 0.70f);
        c[ImGuiCol_Button] = ImVec4(0.78f, 0.25f, 0.50f, 0.55f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.78f, 0.25f, 0.50f, 0.75f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.66f, 0.16f, 0.40f, 0.85f);
        c[ImGuiCol_FrameBg] = ImVec4(1.00f, 0.97f, 0.99f, 1.00f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.93f, 0.80f, 0.88f, 1.00f);
        c[ImGuiCol_CheckMark] = ImVec4(0.72f, 0.10f, 0.38f, 1.00f);
        break;
    }
    case 8: {
        ImGui::StyleColorsDark();
        auto &c = style.Colors;
        c[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.11f, 0.08f, 1.00f);
        c[ImGuiCol_ChildBg] = ImVec4(0.18f, 0.13f, 0.09f, 1.00f);
        c[ImGuiCol_Text] = ImVec4(0.96f, 0.90f, 0.82f, 1.00f);
        c[ImGuiCol_Header] = ImVec4(0.42f, 0.27f, 0.16f, 0.75f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.55f, 0.36f, 0.21f, 0.88f);
        c[ImGuiCol_HeaderActive] = ImVec4(0.65f, 0.43f, 0.26f, 0.95f);
        c[ImGuiCol_Button] = ImVec4(0.36f, 0.24f, 0.15f, 0.85f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.34f, 0.22f, 0.95f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.62f, 0.42f, 0.27f, 1.00f);
        c[ImGuiCol_FrameBg] = ImVec4(0.23f, 0.17f, 0.12f, 1.00f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.33f, 0.24f, 0.16f, 1.00f);
        c[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.70f, 0.38f, 1.00f);
        break;
    }
    case 0:
    default:
        ImGui::StyleColorsDark();
        break;
    }

    darkTheme = theme != 1 && theme != 2 && theme != 5;
    currentStyle = ImGui::GetStyle();
}

void Settings::Load() {
    std::ifstream file(CONFIG_FILE);
    if (!file.good()) {
        ImGui::StyleColorsDark();
        currentStyle = ImGui::GetStyle();
        return;
    }

    recentFiles.clear();

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        const auto separator = line.find('=');
        if (separator == std::string::npos)
            continue;

        const std::string key = trim(line.substr(0, separator));
        const std::string value = trim(line.substr(separator + 1));

        if (key == "dark_theme") {
            darkTheme = parseBool(value, darkTheme);
        } else if (key == "size_font") {
            try {
                sizeFont = std::stof(value);
            } catch (const std::exception &) {
                sizeFont = 24.0f;
            }
        } else if (key == "theme") {
            try {
                theme = std::clamp(std::stoi(value), 0, GetThemeCount() - 1);
            } catch (const std::exception &) {
                theme = darkTheme ? 0 : 1;
            }
        } else if (key == "recent_file") {
            recentFiles.push_back(value);
        }
    }

    if (theme < 0 || theme >= GetThemeCount()) {
        theme = darkTheme ? 0 : 1;
    }
    ApplyTheme();
}

void Settings::Save() {
    // Сохраняем историю файлов (например, последние 10 файлов)
    const size_t maxHistory = 10;
    if (recentFiles.size() > maxHistory) {
        recentFiles.erase(recentFiles.begin(), recentFiles.end() - maxHistory);
    }

    std::ofstream file(CONFIG_FILE);
    file << "# Payroll settings\n";
    file << "dark_theme=" << (darkTheme ? "true" : "false") << "\n";
    file << "theme=" << theme << "\n";
    file << "size_font=" << sizeFont << "\n";
    for (const auto &recentFile : recentFiles) {
        file << "recent_file=" << recentFile << "\n";
    }
}

// Добавление файла в историю
void Settings::AddToHistory(const std::string &filepath) {

    // Удаляем дубликаты
    auto it = std::find(recentFiles.begin(), recentFiles.end(), filepath);
    if (it != recentFiles.end()) {
        recentFiles.erase(it);
    }

    // Добавляем в конец
    recentFiles.push_back(filepath);

    // Сохраняем изменения
    Save();
}

// Переключение темы
void Settings::ToggleTheme() {
    SetTheme(darkTheme ? 1 : 0);
}

void Settings::SetTheme(int newTheme) {
    theme = std::clamp(newTheme, 0, GetThemeCount() - 1);
    ApplyTheme();
    Save();
}

void Settings::getDataFromDB(Database &db) {

    // Загружаем данные из БД нужна 1 строка
    // дублирование с панелью, но хз как сделать

    std::vector<SettingDB> settingsDB;
    const char *sql =
        "SELECT id, organization, note FROM Settings WHERE id = ? LIMIT 1;";
    sqlite3_exec(
        db.getHandle(), sql,
        [](void *data, int argc, char **argv, char **) {
            auto *list = static_cast<std::vector<SettingDB> *>(data);
            // не забываем проверять текстовые поля на NULL
            list->emplace_back(
                SettingDB{std::stoi(argv[0]), argv[1], argv[2] ? argv[2] : ""});
            return 0;
        },
        &settingsDB, nullptr);
    // пока только наименование организации
    organization = settingsDB[0].organization;
}
