
#include <memory>
#include <vector>
#include "EmployeeListPanel.h"
#include "EmployeeEditPanel.h"

class GUI {
public:
    GUI(Database& db);
    void addEmployeePanel();
    void render();

private:
    Database& db;
    std::vector<std::unique_ptr<Panel>> panels;
    void showMainMenu();
};
