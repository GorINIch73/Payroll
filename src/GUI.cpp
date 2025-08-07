#include "GUI.h"
#include "DivisionsPanel.h"
#include "Icons.h"
#include "Database.h"
#include "EmployeesPanel.h"
#include "IndividualsPanel.h"
#include "OrdersPanel.h"
#include "Panel.h"
#include "PositionsPanel.h"
#include "DivisionsPanel.h"
#include "AccrualsPanel.h"
#include "StatementsPanel.h"
#include "SettingsPanel.h"
#include "Manager.h"

#include "ImGuiFileDialog.h"
#include "PdfExporter.h"

#include <iostream>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <utility>

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

    // настройка внешнего вида панели файлового диалога - иконки для файлов и каталогов
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".db",
                                              ImVec4(0.0f, 1.0f, 1.0f, 0.9f),
                                              ICON_FA_DATA_BASE);
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


    ImGui::Begin("Main Window", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings);

    showMainMenu();

    if (showAbout)
        ShowAbout();
    if (showFileDialog)
        ShowFileDialog();



    // Основное окно с табами
    //
    //
    if (ImGui::BeginTabBar("MainTabBar")) {
        // Вкладки
        

        for (size_t i = 0; i < manager_panels.getSize();) {
            Panel* panel = manager_panels.getPanel(i);
            if (!panel) {  // Проверка на nullptr (на всякий случай)
                std::cerr << "Null pointer at Panel" << __LINE__ << std::endl;
                break;
            }
            

            std::string name;
            name = name + std::to_string(i) + " " + panel->getName();
            // std::string name = "таб " + std::to_string(numTab);
            
            // Устанавливаем флаг выбора для последней вкладки
            ImGuiTabItemFlags flags =  0;
            if ((i == manager_panels.getSize()-1) && manager_panels.getNextEnd()) {
                manager_panels.getNextEnd() = false; 
                // goEndPanel = false; 
                flags = ImGuiTabItemFlags_SetSelected;
                // std::cout << "go last panels " << panel->getName() << std::endl;
            }

            if (ImGui::BeginTabItem(name.c_str(), &panel->getIsOpen(),
                                    ImGuiTabItemFlags_UnsavedDocument | flags )) {

                // std::cout << " - render start ... " ;
                panel->render();
                // std::cout << " -- done" << std::endl;
                ImGui::EndTabItem();
            }
            if (!panel->getIsOpen()) {
                printf("%s close\n",panel->getName());
                manager_panels.removePanel(i);
                // Действие при закрытии Tab

            } else {
                ++i;
            }

        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}


void GUI::addEmployeesPanel() {
// сотрудники
    auto newPanel = std::make_unique<EmployeesPanel>(db);
    manager_panels.addPanel(std::move(newPanel));
    manager_panels.getNextEnd()=true;
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
    manager_panels.getNextEnd()=true;
}

void GUI::addSettingsPanel() {
    // Добавляем пнель настроек
    auto newPanel = std::make_unique<SettingsPanel>(db);
    manager_panels.addPanel(std::move(newPanel));
    manager_panels.getNextEnd()=true;
}

void GUI::showMainMenu() {
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("Файл")) {
            
            if (ImGui::MenuItem("Открыть базу")) {
                showFileDialog = true;
                // используем ImGuiFileDialog
            }
            if (ImGui::MenuItem("Сохранить как")) {
                // showFileDialog = true;
                // закрыть все окна
                // запросить новое имя
                // продублировать базу, открыть
            }
            if (ImGui::MenuItem("Закрыть базу")) {
                // перед закрытием комитнуть изменения, закрыть все окна
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
                // не сделано - запросить новое имя

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
        if (ImGui::MenuItem("Настройки")) { 
            addSettingsPanel();
        }
        if (ImGui::MenuItem("О программе")) { /*showAbout(); */
        }
        ImGui::EndMainMenuBar();
    }
}

// сделать диалоги для -открыть, -сохранить как, -создать новую.  
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

// переделано - поправить
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
