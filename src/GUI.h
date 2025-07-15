#pragma ones
#include "Database.h"
#include "Settings.h"
// #include "EmployeesPanel.h"
// #include "IndividualsPanel.h"
#include "Panel.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <memory>
#include <vector>

// #include "EmployeeEditPanel.h"

class GUI {
    public:
        GUI(GLFWwindow *w);
         //void Init(GLFWwindow *window); // Принимаем окно
        void render();

    private:
        GLFWwindow *window;
        Database db;
        // std::vector<std::unique_ptr<Panel>> panels;


        Settings settings;
        std::vector<std::string> recentFiles;
        bool showSettings = false;
        bool showAbout = false;
        bool showFileDialog = false;


        void addEmployeesPanel();
        void addIndividualsPanel();
        void addPositionsPanel();

        void showMainMenu();
        void ShowSettings();
        void ShowAbout();
        void ShowFileDialog();
        void GeneratePdfReport();
};
