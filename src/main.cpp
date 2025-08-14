
#include "Database.h"
#include "EmployeesPanel.h"
#include "GUI.h"
#include "Panel.h"

#include <GLFW/glfw3.h>
// #include <fontconfig/fontconfig.h> // Добавьте этот заголовочный файл
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

int main() {
    // Инициализация fontconfig (должна быть ДО создания ImGui-контекста)
    // if (!FcInit()) {
    //     fprintf(stderr, "Failed to initialize FontConfig\n");
    //     return -1;
    // }
    // Инициализация GLFW и OpenGL
    glfwInit();
    GLFWwindow *window =
        glfwCreateWindow(1280, 720, "Payroll App", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Инициализация ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ImGuiIO &io = ImGui::GetIO();
    // (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //
    // // грузим русский шрифт
    // ImFont *font =
    //     io.Fonts->AddFontFromFileTTF("NotoSans-Regular.ttf", 24.0f, nullptr,
    //                                  io.Fonts->GetGlyphRangesCyrillic());
    //
    // // Добавляем иконки
    // ImFontConfig config;
    // // Добавляем Font Awesome
    // config.MergeMode = true;
    // config.PixelSnapH = true;
    //
    // static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    // io.Fonts->AddFontFromFileTTF("fontawesome-webfont.ttf", 24.0f, &config,
    //                              icons_ranges);
    //
    // // Инициализация базы данных
    // Database db;
    // if (!db.open("payroll.db")) {
    //     std::cerr << "Не удалось открыть БД!" << std::endl;
    //     return -1;
    // }

    // if (!db.createNewDatabase()) {
    //     std::cerr << "Ошибка создания БД!" << std::endl;
    //     return -1;
    // }
    //
    // Основной GUI
    GUI gui(window);

    // Главный цикл
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // ImGui::SetNextWindowPos(ImVec2(0, 0));
        // ImGui::SetNextWindowSize(io.DisplaySize);
        //

        // сотрудники

        // auto newPanel = std::make_unique<EmployeesPanel>(db);
        // manager_panels.push_back(std::move(newPanel));
        gui.render();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Очистка
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
