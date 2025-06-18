
#include "GUI.h"
#include "Database.h"
#include <imgui.h>
#include <iostream>

#include <imgui_internal.h>

GUI::GUI(GLFWwindow *w, Database &base)
    : window(w),
      db(base) {

    // db = base;
    // Создаем панели при старте
    //    auto listPanel = std::make_unique<EmployeeListPanel>(db);
    //    auto editPanel = std::make_unique<EmployeeEditPanel>(db, *listPanel);

    // Добавляем в вектор
    //     panels.push_back(std::move(listPanel));
    //     panels.push_back(std::move(editPanel));
}

void GUI::render() {

    // Получаем размер окна
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

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

    // Основное окно с табами
    //
    // ImGui::SetNextWindowSize(ImGui::GetContentRegionAvail());
    // ImGui::Begin("Основное окно", nullptr,
    //             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
    //                 ImGuiWindowFlags_NoCollapse);
    //
    if (ImGui::BeginTabBar("MainTabBar")) {
        // Вкладка "Сотрудники"
        int numTab = 0;
        for (auto panel = panels.begin(); panel != panels.end();) {

            std::string name = "Вкладка " + std::to_string(numTab);

            if (ImGui::BeginTabItem(name.c_str(), &(*panel)->getIsOpen(),
                                    ImGuiTabItemFlags_UnsavedDocument)) {

                (*panel)->render();
                ImGui::EndTabItem();
            }
            if (!(*panel)->getIsOpen()) {
                panel = panels.erase(panel);
                // Действие при закрытии Tab
                printf("Tab 1 был закрыт\n");

            } else {
                ++panel;
            }

            numTab++;
        }
        // // Вкладка "Начисления"
        // if (ImGui::BeginTabItem("Начисления")) {
        //     // Здесь можно разместить панель для работы с начислениями
        //     ImGui::Text("Панель начислений будет здесь");
        //     ImGui::EndTabItem();
        // }
        //
        // // Вкладка "Отчеты"
        // if (ImGui::BeginTabItem("Отчеты")) {
        //     ImGui::Text("Панель отчетов будет здесь");
        //     ImGui::EndTabItem();
        // }
        //
        ImGui::EndTabBar();
    }

    ImGui::End();
    // Рендерим все активные панели
}

void GUI::addEmployeesPanel() {

    auto newPanel = std::make_unique<EmployeesPanel>(db);

    // auto newEditPanel = std::make_unique<EmployeeEditPanel>(db,
    // *newListPanel);

    // auto newEditPanel = std::make_unique<EmployeeEditPanel>(db,
    //     *static_cast<EmployeeListPanel*>(panels[0].get()));

    //    newPanel->setEmployee(Employee{0, "", "", 0.0});  // Новый сотрудник

    newPanel->getIsOpen() = true;
    // newEditPanel->getIsOpen() = true;

    panels.push_back(std::move(newPanel));
    // panels.push_back(std::move(newEditPanel));
}

void GUI::showMainMenu() {
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("Файл")) {
            if (ImGui::MenuItem("Создать БД")) {
                /* Создать новую БД */

                if (!db.createNewDatabase()) {
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
                // for (auto& panel : panels) {
                //     if (panel->getName() == "Справочник сотрудников") {
                //         panel->getIsOpen() = true;
                //         break;
                //     }
                // }
            }
            if (ImGui::MenuItem("Редактор")) {
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
