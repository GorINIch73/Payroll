
#include "Database.h"
#include "EmployeesPanel.h"
#include "GUI.h"
#include "Panel.h"
#include "Settings.h"

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

    // Основной GUI
    GUI gui(window);
    settings.Load();
    // Главный цикл
    int display_w, display_h;
    while (!glfwWindowShouldClose(window)) {
        // glfwPollEvents();

        // Проверяем состояние окна
        bool isFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED);
        bool isMinimized = glfwGetWindowAttrib(window, GLFW_ICONIFIED);

        // Обработка событий в зависимости от состояния
        if (isMinimized) {
            glfwWaitEvents(); // Минимальная нагрузка
            continue;
        }

        if (!isFocused) {
            // Фоновый режим - ограничиваем FPS
            glfwWaitEventsTimeout(0.1); // ~10 FPS
        } else {
            glfwWaitEventsTimeout(0.033); // ~30 FPS
        }

        // если надо грузим шрифт - не работает динамически - хз почему
        if (settings.needLoadFonts)
            settings.LoadFonts();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        // рендерим окна
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
