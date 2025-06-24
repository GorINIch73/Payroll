#pragma ones
#include "Database.h"
#include "EmployeesPanel.h"
#include "IndividualsPanel.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <memory>
#include <vector>

// #include "EmployeeEditPanel.h"

class GUI {
    public:
        GUI(GLFWwindow *w, Database &base);

        void addEmployeesPanel();
        void addIndividualsPanel();

        void render();
        void showMainMenu();

    private:
        GLFWwindow *window;
        Database &db;
        std::vector<std::unique_ptr<Panel>> panels;
};
