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

// extern std::vector<std::unique_ptr<Panel>> manager_panels;

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
        // std::vector<std::string> recentFiles;
        bool showSettings = false;
        bool showAbout = false;
        bool showFileDialogOpen = false;
        bool showFileDialogNew = false;
        bool showFileDialogSaveAs = false;
        // bool goEndPanel  = false;


        void addEmployeesPanel();
        void addIndividualsPanel();
        void addPositionsPanel();
        void addDivisionsPanel();
        void addAccrualsPanel();
        void addOrdersPanel();
        void addStatementsPanel();
        void addSettingsPanel();

        void showMainMenu();
        void ShowSettings();
        void ShowAbout();
        void ShowFileDialogOpen();
        void ShowFileDialogNew();
        void ShowFileDialogSaveAs();
        void GeneratePdfReport();
};
