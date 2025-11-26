
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
#include <string>

// Callback для обновления размеров ImGui при изменении окна
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // Обновляем viewport
    glViewport(0, 0, width, height);

    ImGuiIO &io = ImGui::GetIO();
    int win_width, win_height;
    glfwGetWindowSize(window, &win_width, &win_height);

    if (win_width > 0 && win_height > 0) {
        io.DisplayFramebufferScale =
            ImVec2(width / (float)win_width, height / (float)win_height);
    }
}

int main() {
    // Инициализация fontconfig (должна быть ДО создания ImGui-контекста)
    // if (!FcInit()) {
    //     fprintf(stderr, "Failed to initialize FontConfig\n");
    //     return -1;
    // }
    // Инициализация GLFW и OpenGL
    // без этого прыгала и не всегда срабатывала мышь
    glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_DISABLE_LIBDECOR);
    glfwInit();
    GLFWwindow *window =
        glfwCreateWindow(1280, 720, "Payroll App", nullptr, nullptr);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync
    //

    // Устанавливаем callback для изменений размеров
    // glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Инициализация ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // тайминги для лучшего распознавания кликов
    io.MouseDoubleClickTime = 0.5f;    // Увеличьте если нужно
    io.MouseDoubleClickMaxDist = 8.0f; // Немного увеличьте радиус
    //
    // Для быстрых кликов важно уменьшить задержку
    io.ConfigInputTrickleEventQueue = false; // Отключите если включено
                                             // //
                                             // // io.MouseDrawCursor = false;
    // LOG_DEBUG_MSG("Go...");
    // Получаем текущие размеры framebuffer'а для начальной настройки
    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    int win_width, win_height;
    glfwGetWindowSize(window, &win_width, &win_height);

    // Настраиваем масштаб DPI
    io.DisplaySize = ImVec2((float)fb_width, (float)fb_height);
    io.DisplayFramebufferScale =
        ImVec2(fb_width / (float)win_width, fb_height / (float)win_height);

    // Инициализация рендерера
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
        // что то не так с поведенгием мыши - надо
        // посмотреть!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! glfwWaitEvents(); //
        // Минимальная нагрузка Обработка событий в зависимости от состояния
        if (isMinimized) {
            glfwWaitEvents(); // Минимальная нагрузка
            continue;
        }

        // glfwPollEvents();
        if (!isFocused) {
            // Фоновый режим - ограничиваем FPS
            glfwWaitEventsTimeout(0.1); // ~10 FPS
        } else {
            glfwWaitEventsTimeout(0.033); // ~30 FPS
        }

        // если надо грузим шрифт - не работает динамически - хз почему
        if (settings.needLoadFonts)
            settings.LoadFonts();

        // Получаем актуальные размеры framebuffer'а
        int display_w, display_h;
        // glfwGetFramebufferSize(window, &display_w, &display_h);
        glfwGetWindowSize(window, &display_w, &display_h);

        // Настраиваем ImGui на новый кадр с правильными размерами
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        // Устанавливаем правильные размеры дисплея
        // ImGuiIO &io = ImGui::GetIO();
        // не работает установка размера или установка масштабироания - пока
        // отключил
        // io.DisplaySize = ImVec2((float)display_w, (float)display_h);
        // io.DisplaySize = ImVec2((float)500, (float)500);

        // g_MessageLog.Add(std::to_string((float)display_w) + "-" +
        //                  std::to_string((float)display_h));
        // g_MessageLog.Add(std::to_string(display_w) + "-" +
        //                  std::to_string(display_h));
        // g_MessageLog.Add(std::to_string(display_h));
        ImGui::NewFrame();
        // ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        //
        // ImGui::NewFrame();

        // ImGui::ShowDemoWindow();
        // рендерим окна
        //

        // glfwPollEvents();

        // glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
        // glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

        gui.render();

        ImGui::Render();
        // int display_w, display_h;
        // glfwGetFramebufferSize(window, &display_w, &display_h);
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
