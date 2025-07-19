#include "GUI.h"
#include "Icons.h"
#include "Database.h"
#include "EmployeesPanel.h"
#include "IndividualsPanel.h"
#include "Panel.h"
#include "PositionsPanel.h"

#include "ImGuiFileDialog.h"
#include "PdfExporter.h"

#include <iostream>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// #include <imgui_internal.h>

GUI::GUI(GLFWwindow *w)
    : window(w)
       {

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

    // Инициализация базы данных
      // Database db;
    // if (!db.open("payroll.db")) {
    //     std::cerr << "Не удалось открыть БД!" << std::endl;
    //     // return -1;
    // }

    // ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".png",
    //                                           ImVec4(0.0f, 1.0f, 1.0f, 0.9f),
    //                                           ICON_FA_FILE_IMAGE);
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".db",
                                              ImVec4(0.0f, 1.0f, 1.0f, 0.9f),
                                              ICON_FA_DATA_BASE);
    // ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".txt",
    //                                           ImVec4(0.0f, 1.0f, 1.0f, 0.9f),
    //                                           ICON_FA_FILE_LINES);
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "",
                                              ImVec4(0.0f, 1.0f, 1.0f, 0.9f),
                                              ICON_FA_FOLDER); // Папки

    // загружаем данные из конфига
    settings.Load("config.json");
    //	std::cout << "old file: " << settings.lastDbPath;
    recentFiles.push_back(settings.lastDbPath);

}

void GUI::render() {

// Получаем размер окна
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    // ImGui::SetNextWindowViewport(viewport->ID);

    // ImVec2 parent_size = ImGui::GetWindowSize();
    // ImGui::SetNextWindowPos(ImVec2(0, 0));
    // ImGui::SetNextWindowSize(parent_size);
    // // ImGui::Begin("Fullscreen Child", nullptr,
    // ImGuiWindowFlags_NoCollapse);
    // ImGui::Begin("Payroll", nullptr,
    //              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
    //                  ImGuiWindowFlags_NoMove |
    //                  ImGuiWindowFlags_NoBringToFrontOnFocus |
    //                  ImGuiWindowFlags_MenuBar);
    //

    ImGui::Begin("Main Window", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings);

    showMainMenu();

    if (showSettings)
        ShowSettings();
    if (showAbout)
        ShowAbout();
    if (showFileDialog)
        ShowFileDialog();



    // Основное окно с табами
    //
    // ImGui::SetNextWindowSize(ImGui::GetContentRegionAvail());
    // ImGui::Begin("Основное окно", nullptr,
    //             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
    //                 ImGuiWindowFlags_NoCollapse);
    //
    if (ImGui::BeginTabBar("MainTabBar")) {
        // Вкладки
        
        int numTab = 0;
        for (auto panel = manager_panels.begin(); panel != manager_panels.end();) {

            std::string name;
            name = name + std::to_string(numTab) + " " + (*panel)->getName();
            // std::string name = "таб " + std::to_string(numTab);
            
            // Устанавливаем флаг выбора для последней вкладки
            ImGuiTabItemFlags flags =  0;
            if (goEndPanel && (panel == std::prev(manager_panels.end()))) {
                goEndPanel = false;
                flags = ImGuiTabItemFlags_SetSelected;
                // std::cout << "go last panels " << (*panel)->getName() << std::endl;
            }

            if (ImGui::BeginTabItem(name.c_str(), &(*panel)->getIsOpen(),
                                    ImGuiTabItemFlags_UnsavedDocument | flags )) {

                (*panel)->render();
                ImGui::EndTabItem();
            }
            if (!(*panel)->getIsOpen()) {
                printf("%s был закрыт\n",(*panel)->getName());
                panel = manager_panels.erase(panel);
                // Действие при закрытии Tab

            } else {
                ++panel;
            }

            numTab++;

        }
        ImGui::EndTabBar();
    }

    ImGui::End();
    // Рендерим все активные панели
}

void GUI::addEmployeesPanel() {
// сотрудники
    auto newPanel = std::make_unique<EmployeesPanel>(db);
    newPanel->getIsOpen() = true;

    manager_panels.push_back(std::move(newPanel));
    goEndPanel = true;
}

void GUI::addPositionsPanel() {
    // Добавляем пнель должностей
    auto newPanel = std::make_unique<PositionsPanel>(db);
    newPanel->getIsOpen() = true;

    manager_panels.push_back(std::move(newPanel));
    goEndPanel = true;
}

void GUI::addIndividualsPanel() {
    // Добавляем пнель физлиц
    auto newPanel = std::make_unique<IndividualsPanel>(db);
    newPanel->getIsOpen() = true;

    manager_panels.push_back(std::move(newPanel));
    goEndPanel = true;
}

void GUI::showMainMenu() {
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("Файл")) {
            
            if (ImGui::MenuItem("Открыть базу")) {
                showFileDialog = true;
                // Логика открытия файла (используем ImGuiFileDialog)
            }
            if (ImGui::MenuItem("Закрыть базу")) {
                db.Close();
            }

            if (ImGui::BeginMenu("Последние файлы")) {
                for (const auto &file : recentFiles) {
                    if (ImGui::MenuItem(file.empty() ? "путо" :  file.c_str())) {
                        db.Open(file);
                    }
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::MenuItem("Создать БД")) {
                /* Создать новую БД */

                if (!db.CreateNewDatabase()) {
                    std::cerr << "Ошибка создания БД!" << std::endl;
                    return;
                }
            }
            if (ImGui::MenuItem("Выход")) {
                /* Закрыть приложение */
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Справочники")) {
            if (ImGui::MenuItem("Сотрудники")) {
                addEmployeesPanel();
                // Ищем панель в списке и открываем
                // for (auto& panel : manager_panels) {
                //     if (panel->getName() == "Справочник сотрудников") {
                //         panel->getIsOpen() = true;
                //         break;
                //     }
                // }
            }
            if (ImGui::MenuItem("Физлица")) {
                addIndividualsPanel();
            }
            if (ImGui::MenuItem("Должности")) {
                addPositionsPanel();
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Отчет")) { /* ... */
        }
        if (ImGui::MenuItem("Настройки")) { /* ... */
        }
        if (ImGui::MenuItem("О программе")) { /*showAbout(); */
        }
        ImGui::EndMainMenuBar();
    }
}

  
void GUI::ShowFileDialog() {
    if (!showFileDialog)
        return;

    IGFD::FileDialogConfig config;
    config.path = ".";
    ImGuiFileDialog::Instance()->OpenDialog(
        "ChooseFileDlg", // Уникальный ID
        "Выберите файл", // Заголовок
        //            ".*,.db,.pdf",     // Фильтры (через запятую)
        ".db", // Фильтр (через запятую)
        config // Стартовая папка
    );

    // Обработка выбора файла
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlg")) {
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
            if (!db.Open(filePathName)) {
                std::cerr << "Не удалось открыть БД!" << std::endl;
                // return -1;
            }
            else {
                // Добавляем в список последних файлов и сохраняем
                recentFiles.push_back(filePathName);
                settings.lastDbPath = filePathName;
                settings.Save("config.json");
            }
        }

        // Закрываем диалог
        ImGuiFileDialog::Instance()->Close();
        showFileDialog = false;
    }
}

void GUI::ShowSettings() {
    if (!showSettings)
        return;

    ImGui::Begin("Настройки", &showSettings);
    ImGui::Combo("Тема", &settings.theme, "Светлая\0Темная\0");

    if (ImGui::Button("Сохранить")) {
        settings.Save("config.json");
    }
    ImGui::End();
}

void GUI::ShowAbout() {
    if (!showAbout)
        return;

    ImGui::Begin("Об авторах", &showAbout);

    ImGui::End();
}

void GUI::GeneratePdfReport() {
    auto data = db.FetchAll("SELECT * FROM users");
    PdfExporter exporter;
    exporter.ExportToPdf(data, "report.pdf");
}
