#include "GUI.h"
#include "AccrualsPanel.h"
#include "Database.h"
#include "DivisionsPanel.h"
#include "EmployeesPanel.h"
#include "Icons.h"
#include "IndividualsPanel.h"
#include "Manager.h"
#include "OrdersPanel.h"
#include "Panel.h"
#include "PositionsPanel.h"
#include "SettingsPanel.h"
#include "StatementsPanel.h"

#include "ImGuiFileDialog.h"
#include "PdfExporter.h"
#include "imgui_components.h"

#include <iostream>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
// #include <strstream>
#include <utility>

// #include <imgui_internal.h>

GUI::GUI(GLFWwindow *w)
    : window(w) {

    // db = base;
    // Создаем панели при старте
    //    auto listPanel = std::make_unique<EmployeeListPanel>(db);
    //    auto editPanel = std::make_unique<EmployeeEditPanel>(db, *listPanel);

    // Добавляем в вектор
    //     manager_panels.push_back(std::move(listPanel));
    //     manager_panels.push_back(std::move(editPanel));

    // Настройка иконок в ImGuiFileDialog

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // грузим русский шрифт
    ImFont *font =
        io.Fonts->AddFontFromFileTTF("NotoSans-Regular.ttf", 24.0f, nullptr,
                                     io.Fonts->GetGlyphRangesCyrillic());

    // Добавляем иконки
    ImFontConfig config;
    // Добавляем Font Awesome
    config.MergeMode = true;
    config.PixelSnapH = true;

    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    io.Fonts->AddFontFromFileTTF("fontawesome-webfont.ttf", 24.0f, &config,
                                 icons_ranges);

    // настройка внешнего вида панели файлового диалога - иконки для файлов и
    // каталогов
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".db",
                                              ImVec4(0.0f, 1.0f, 1.0f, 0.9f),
                                              ICON_FA_DATA_BASE);
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "",
                                              ImVec4(0.0f, 1.0f, 1.0f, 0.9f),
                                              ICON_FA_FOLDER); // Папки

    // загружаем данные из конфига
    settings.Load();
    g_MessageLog.Add("Добро пожаловать.");
}

void GUI::render() {

    // Получаем размер окна
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::Begin("Main Window", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings);

    showMainMenu();

    ShowAbout();
    ShowClearDB();

    if (showFileDialogOpen)
        ShowFileDialogOpen();
    if (showFileDialogNew)
        ShowFileDialogNew();
    if (showFileDialogSaveAs)
        ShowFileDialogSaveAs();

    // Основное окно с табами
    //
    //

    ImGui::BeginChild(
        "Main",
        ImVec2(0, ImGui::GetContentRegionAvail().y - g_MessageLog.height),
        true);
    {
        if (ImGui::BeginTabBar("MainTabBar")) {
            // Вкладки

            for (size_t i = 0; i < manager_panels.getSize();) {
                Panel *panel = manager_panels.getPanel(i);
                if (!panel) { // Проверка на nullptr (на всякий случай)
                    std::cerr << "Null pointer at Panel" << __LINE__
                              << std::endl;
                    break;
                }

                std::string name;
                name = name + std::to_string(i) + " " + panel->getName();
                // std::string name = "таб " + std::to_string(numTab);

                // Устанавливаем флаг выбора для последней вкладки
                ImGuiTabItemFlags flags = 0;
                if ((i == manager_panels.getSize() - 1) &&
                    manager_panels.getNextEnd()) {
                    manager_panels.getNextEnd() = false;
                    // goEndPanel = false;
                    flags = ImGuiTabItemFlags_SetSelected;
                    // std::cout << "go last panels " << panel->getName() <<
                    // std::endl;
                }

                if (ImGui::BeginTabItem(name.c_str(), &panel->getIsOpen(),
                                        ImGuiTabItemFlags_UnsavedDocument |
                                            flags)) {

                    // std::cout << " - render start ... " ;
                    panel->render();
                    // std::cout << " -- done" << std::endl;
                    ImGui::EndTabItem();
                }
                if (!panel->getIsOpen()) {
                    printf("%s close\n", panel->getName());
                    manager_panels.removePanel(i);
                    // Действие при закрытии Tab

                } else {
                    ++i;
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::EndChild();
        g_MessageLog.Draw(); // статусбар
    }
    ImGui::End();
    // g_MessageLog.Draw(); // статусбар
}

void GUI::addEmployeesPanel() {
    // сотрудники
    auto newPanel = std::make_unique<EmployeesPanel>(db);
    manager_panels.addPanel(std::move(newPanel));
    manager_panels.getNextEnd() = true;
}

void GUI::addPositionsPanel() {
    // Добавляем пнель должностей
    auto newPanel = std::make_unique<PositionsPanel>(db);
    manager_panels.addPanel(std::move(newPanel));
    manager_panels.getNextEnd() = true;
}

void GUI::addAccrualsPanel() {
    // Добавляем пнель рачислений
    auto newPanel = std::make_unique<AccrualsPanel>(db);
    manager_panels.addPanel(std::move(newPanel));
    manager_panels.getNextEnd() = true;
}

void GUI::addDivisionsPanel() {
    // Добавляем пнель отделенй
    auto newPanel = std::make_unique<DivisionsPanel>(db);
    manager_panels.addPanel(std::move(newPanel));
    manager_panels.getNextEnd() = true;
}

void GUI::addOrdersPanel() {
    // Добавляем пнель отделенй
    auto newPanel = std::make_unique<OrdersPanel>(db);
    manager_panels.addPanel(std::move(newPanel));
    manager_panels.getNextEnd() = true;
}

void GUI::addStatementsPanel() {
    // Добавляем пнель ведомостей
    auto newPanel = std::make_unique<StatementsPanel>(db);
    manager_panels.addPanel(std::move(newPanel));
    manager_panels.getNextEnd() = true;
}

void GUI::addIndividualsPanel() {
    // Добавляем пнель физлиц
    auto newPanel = std::make_unique<IndividualsPanel>(db);
    manager_panels.addPanel(std::move(newPanel));
    manager_panels.getNextEnd() = true;
}

void GUI::addSettingsPanel() {
    // Добавляем пнель настроек
    auto newPanel = std::make_unique<SettingsPanel>(db);
    manager_panels.addPanel(std::move(newPanel));
    manager_panels.getNextEnd() = true;
}

void GUI::showMainMenu() {
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("Файл")) {

            if (ImGui::MenuItem("Открыть базу")) {
                showFileDialogOpen = true;
                // используем ImGuiFileDialog
            }
            if (ImGui::MenuItem("Сохранить как")) {
                if (db.IsOpen())
                    // закрыть все окна ХЗ
                    showFileDialogSaveAs = true;
                else
                    g_MessageLog.Add("Нечего сохранять. База не открыта!",
                                     ImVec4(0.6, 0.3, 0.3, 1));
            }
            if (ImGui::MenuItem("Закрыть базу")) {
                // перед закрытием комитнуть изменения, закрыть все окна
                db.Close();
            }

            if (ImGui::MenuItem("Создать новую базу")) {
                /* Создать новую БД */
                // не сделано - запросить новое имя
                showFileDialogNew = true;

                // if (!db.CreateNewDatabase()) {
                //     std::cerr << "Ошибка создания БД!" << std::endl;
                //     return;
                // }
            }

            if (ImGui::BeginMenu("Последние файлы")) {

                // Отображаем в обратном порядке (последние - сверху)
                for (auto it = settings.recentFiles.rbegin();
                     it != settings.recentFiles.rend(); ++it) {
                    // if (ImGui::MenuItem(it.empty() ? "" : it->c_str())) {
                    if (ImGui::MenuItem(it->c_str())) {
                        // Обработка выбора файла из истории
                        // закрываем все панели, они сохранят данные сами
                        manager_panels.removeAllPanels();
                        db.Close();
                        std::string file_o = it->c_str();
                        if (!db.Open(file_o)) {
                            std::cerr << "Не удалось открыть БД!" << std::endl;
                            // return -1;
                            g_MessageLog.Add("Не удалось открыть базу: " +
                                                 file_o,
                                             ImVec4(0.6, 0.3, 0.3, 1));
                        } else {
                            g_MessageLog.Add("База: " + file_o + " открыта.",
                                             ImVec4(0.3, 0.6, 0.3, 1));
                        }
                    }
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Очистить историю")) {
                    settings.recentFiles.clear();
                    settings.Save();
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Выход")) {
                /* Закрыть приложение */
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Справочники")) {
            if (ImGui::MenuItem(ICON_FA_LIST_ALT " Ведомости")) {
                addStatementsPanel();
            }
            if (ImGui::MenuItem(ICON_FA_GROUP " Сотрудники")) {
                addEmployeesPanel();
            }
            if (ImGui::MenuItem(ICON_FA_ORDER " Приказы")) {
                addOrdersPanel();
            }
            if (ImGui::MenuItem(ICON_FA_USER " Физлица")) {
                addIndividualsPanel();
            }
            if (ImGui::MenuItem(ICON_FA_TIE " Должности")) {
                addPositionsPanel();
            }
            if (ImGui::MenuItem(ICON_FA_DIVISIONS " Отделения")) {
                addDivisionsPanel();
            }
            if (ImGui::MenuItem(ICON_FA_MONEY " Начисления")) {
                addAccrualsPanel();
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Отчет")) { /* ... */
        }

        if (ImGui::BeginMenu("Разное")) {
            if (ImGui::MenuItem("Настройки")) {
                addSettingsPanel();
            }
            if (ImGui::MenuItem("Очистка базы")) {
                showClearDB = true;
            }

            ImGui::Separator();
            if (ImGui::MenuItem(settings.darkTheme ? "Light Theme"
                                                   : "Dark Theme")) {
                settings.ToggleTheme();
            }
            ImGui::Separator();

            if (ImGui::MenuItem("О программе")) { /*showAbout(); */
                showAbout = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

// сделать диалоги для -открыть, -сохранить как, -создать новую.
void GUI::ShowFileDialogOpen() {
    if (!showFileDialogOpen)
        return;

    IGFD::FileDialogConfig config;
    config.path = ".";
    config.filePathName = "";                  // Начальное имя файла
    config.flags = ImGuiFileDialogFlags_Modal; // Делаем диалог модальным

    ImGuiFileDialog::Instance()->OpenDialog(
        "ChooseFileDlgOpen",  // Уникальный ID
        "Выберите файл базы", // Заголовок
        //            ".*,.db,.pdf",     // Фильтры (через запятую)
        ".db", // Фильтр (через запятую)
        config // Стартовая папка
    );

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    // Обработка выбора файла
    if (ImGuiFileDialog::Instance()->Display(
            "ChooseFileDlgOpen",
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove,
            viewport->Size, // Размер - весь вьюпорт
            viewport->Size  // Минимальный размер
            )) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::string filePathName =
                ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath =
                ImGuiFileDialog::Instance()->GetCurrentPath();

            // std::cout << "Выбран файл имя: " << filePathName << std::endl;
            // std::cout << "Выбран файл: " << filePath << std::endl;

            // закрываем все панели, они сохранят данные сами
            manager_panels.removeAllPanels();
            db.Close();
            if (!db.Open(filePathName)) {
                std::cerr << "Не удалось открыть БД!" << std::endl;
                // return -1;
                g_MessageLog.Add("Не удалось открыть базу: " + filePathName,
                                 ImVec4(0.6, 0.3, 0.3, 1));
            } else {
                g_MessageLog.Add("База: " + filePathName + " открыта.",
                                 ImVec4(0.3, 0.6, 0.3, 1));
                // Добавляем в список последних файлов и сохраняем
                settings.AddToHistory(filePathName);
                // recentFiles.push_back(filePathName);
                // settings.lastDbPath = filePathName;
                // settings.Save("config.json");
            }
        }

        // Закрываем диалог
        ImGuiFileDialog::Instance()->Close();
        showFileDialogOpen = false;
    }
}

void GUI::ShowFileDialogNew() {
    if (!showFileDialogNew)
        return;

    IGFD::FileDialogConfig config;
    config.path = ".";
    config.filePathName = "new_base.db"; // Имя по умолчанию
    config.flags =
        ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite;
    ImGuiFileDialog::Instance()->OpenDialog(
        "ChooseFileDlgOpen",       // Уникальный ID
        "Веведите имя новой базы", // Заголовок
        //            ".*,.db,.pdf",     // Фильтры (через запятую)
        ".db", // Фильтр (через запятую)
        config // Стартовая папка
    );

    // Обработка выбора файла
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    if (ImGuiFileDialog::Instance()->Display(
            "ChooseFileDlgOpen",
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove,
            viewport->Size, // Размер - весь вьюпорт
            viewport->Size  // Минимальный размер
            )) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::string filePathName =
                ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath =
                ImGuiFileDialog::Instance()->GetCurrentPath();

            // std::cout << "Выбран файл имя: " << filePathName << std::endl;
            // std::cout << "Выбран файл: " << filePath << std::endl;

            // загрузка базы данных
            // надо сделать принудительное закрытие всех окон - хз

            // db.Open(filePathName);
            // закрываем все панели, они сохранят данные сами
            manager_panels.removeAllPanels();
            db.Close();
            if (!db.Open(filePathName)) {
                std::cerr << "Не удалось открыть БД!" << std::endl;
                g_MessageLog.Add("Не удалось открыть базу: " + filePathName,
                                 ImVec4(0.6, 0.3, 0.3, 1));
                // return -1;
            } else {
                db.CreateNewDatabase();
                g_MessageLog.Add("База " + filePathName + "создана.",
                                 ImVec4(0.5, 0.3, 0.6, 1));
                // Добавляем в список последних файлов и сохраняем
                settings.AddToHistory(filePathName);
                // recentFiles.push_back(filePathName);
                // settings.lastDbPath = filePathName;
                // settings.Save("config.json");
            }
        }

        // Закрываем диалог
        ImGuiFileDialog::Instance()->Close();
        showFileDialogNew = false;
    }
}

void GUI::ShowFileDialogSaveAs() {
    if (!showFileDialogSaveAs)
        return;

    IGFD::FileDialogConfig config;
    config.path = ".";
    config.filePathName = db.getPath(); // Начальное имя файла
    config.flags =
        ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite;

    ImGuiFileDialog::Instance()->OpenDialog(
        "ChooseFileDlgOpen",      // Уникальный ID
        "Задайте новое имя базы", // Заголовок
        //            ".*,.db,.pdf",     // Фильтры (через запятую)
        ".db", // Фильтр (через запятую)
        config // Стартовая папка
    );

    // Обработка выбора файла
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    if (ImGuiFileDialog::Instance()->Display(
            "ChooseFileDlgOpen",
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove,
            viewport->Size, // Размер - весь вьюпорт
            viewport->Size  // Минимальный размер
            )) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::string filePathName =
                ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath =
                ImGuiFileDialog::Instance()->GetCurrentPath();

            // std::cout << "Выбран файл имя: " << filePathName << std::endl;
            // std::cout << "Выбран файл: " << filePath << std::endl;

            // загрузка базы данных
            // надо сделать принудительное закрытие всех окон - хз

            if (filePathName != db.getPath()) {

                if (db.CopyDatabase(filePathName) != SQLITE_OK &&
                    filePathName != db.getPath()) {

                    g_MessageLog.Add("Ошибка: база " + filePathName +
                                         " не скопирована!",
                                     ImVec4(1, 0.3, 0.3, 1));
                } else {
                    // закрываем все панели, они сохранят данные сами
                    manager_panels.removeAllPanels();
                    db.Close();
                    if (!db.Open(filePathName)) {
                        std::cerr << "Не удалось открыть новую БД!"
                                  << std::endl;
                        // return -1;
                    } else {
                        g_MessageLog.Add("Скопированная база: " + filePathName +
                                             " открыта.",
                                         ImVec4(0.3, 0.6, 0.3, 1));
                        // Добавляем в список последних файлов и сохраняем
                        settings.AddToHistory(filePathName);
                        // recentFiles.push_back(filePathName);
                        // settings.lastDbPath = filePathName;
                        // settings.Save("config.json");
                    }
                }
            }
        }

        // Закрываем диалог
        ImGuiFileDialog::Instance()->Close();
        showFileDialogSaveAs = false;
    }
}

// диалог выборочной очистки таблиц
void GUI::ShowClearDB() {
    if (!showClearDB)
        return;

    if (!db.IsOpen()) {
        g_MessageLog.Add("База не открыта!");
        showClearDB = false;
        // ImGui::CloseCurrentPopup();
        return;
    }

    static bool clearStatements = false;
    static bool clearEmployeers = false;
    static bool clearIndividuals = false;
    static bool clearPositions = false;
    static bool clearOrders = false;
    static bool clearAccurals = false;
    static bool clearDivisions = false;

    ImGui::OpenPopup("Clear");
    if (ImGui::BeginPopupModal("Clear", NULL,
                               ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_AlwaysAutoResize)) {

        // рисуем боксы для выбора таблиц
        ImGui::Text("Выберите таблицы для очистки:");
        ImGui::Separator();

        ImGui::Checkbox("Таблица ведомостей", &clearStatements);

        ImGui::Checkbox("Таблица начислений", &clearAccurals);
        ImGui::Checkbox("Таблица приказов", &clearOrders);

        ImGui::Separator();
        ImGui::Checkbox("Таблица сотрудников", &clearEmployeers);
        ImGui::Checkbox("Таблица физлиц", &clearIndividuals);
        ImGui::Checkbox("Таблица должностей", &clearPositions);
        ImGui::Checkbox("Таблица отделений", &clearDivisions);

        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.9f, 0.1f, 0.1f, 1.0f));   // красный
        if (ImGui::Button(ICON_FA_TRASH "Осичтить выбранное")) { /* ... */
            // showConfirmation = true;
            ImGui::OpenPopup("Подтверждение");
        }

        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button("Закрыть")) {
            showClearDB = false;
            ImGui::CloseCurrentPopup();
        }

        // проверяем и очищаем

        if (ImGui::BeginPopupModal("Подтверждение", NULL,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Вы точно хотите очистить выбранные таблицы?");
            ImGui::Text("Это действие нельзя отменить!");
            if (ImGui::Button("  Да  ")) {
                // закрываем все панели, они сохранят данные сами
                manager_panels.removeAllPanels();
                // Очистка выбранных таблиц

                // ведомости
                if (clearStatements) {
                    const char *sql = R"(
                        DELETE FROM List_accruals;
                        DELETE FROM Statements;
                    )";
                    if (db.Execute(sql))
                        g_MessageLog.Add("Ведомости очищены.");
                }

                // справочник начисчслений
                if (clearAccurals) {
                    const char *sql = R"(
                        DELETE FROM Accruals;
                        UPDATE List_accruals SET accrual_id = NULL;
                    )";
                    if (db.Execute(sql))
                        g_MessageLog.Add("Справочник начислений очищен.");
                }

                // справочник приказов
                if (clearOrders) {
                    const char *sql = R"(
                        DELETE FROM Orders;
                        UPDATE List_accruals SET order_id = NULL;
                    )";
                    if (db.Execute(sql))
                        g_MessageLog.Add("Справочник приказов очищен.");
                }

                // справочник сотрудников
                if (clearEmployeers) {
                    const char *sql = R"(
                        DELETE FROM Employees;
                    )";
                    if (db.Execute(sql))
                        g_MessageLog.Add("Справочник сотрудников очищен.");
                }

                // справочник физлиц
                if (clearIndividuals) {
                    const char *sql = R"(
                        DELETE FROM Individuals;
                        UPDATE Employees SET individual_id = NULL;
                    )";
                    if (db.Execute(sql))
                        g_MessageLog.Add("Справочник физлиц очищен.");
                }

                // справочник должностей
                if (clearPositions) {
                    const char *sql = R"(
                        DELETE FROM Positions;
                        UPDATE Employees SET position_id = NULL;
                    )";
                    if (db.Execute(sql))
                        g_MessageLog.Add("Справочник должностей очищен.");
                }

                // справочник отделений
                if (clearDivisions) {
                    const char *sql = R"(
                        DELETE FROM Divisions;
                        UPDATE Employees SET division_id = NULL;
                    )";
                    if (db.Execute(sql))
                        g_MessageLog.Add("Справочник отделений очищен.");
                }

                // g_MessageLog.Add("Очистка завершена.");

                showClearDB = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("  Нет  ")) {
                showClearDB = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        // if (ImGui::Button("Закрыть")) {
        //     ImGui::CloseCurrentPopup();
        //     showClearDB = false;
        // }

        ImGui::EndPopup();
    }
}

void GUI::ShowAbout() {
    if (!showAbout)
        return;

    ImGui::OpenPopup("About");
    if (ImGui::BeginPopupModal("About", NULL,
                               ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::Text("GorINIch`tm 2025 \n ggroinich@gmail.com");
        if (ImGui::Button("Закрыть")) {
            ImGui::CloseCurrentPopup();
            showAbout = false;
        }

        ImGui::EndPopup();
    }
}

void GUI::GeneratePdfReport() {
    auto data = db.FetchAll("SELECT * FROM users");
    PdfExporter exporter;
    exporter.ExportToPdf(data, "report.pdf");
}
