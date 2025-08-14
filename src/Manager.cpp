
#include "Manager.h"
#include "Panel.h"

#include <algorithm> // для std::find_if

#include <cstddef>
#include <iostream>

// Определение вектора
// std::vector<std::unique_ptr<Panel>> manager_panels;

void ManagerPanels::addPanel(std::unique_ptr<Panel> panel) {
    panels.push_back(std::move(panel));
    // std::cout << "add " << manager_panels.size() << " Panel:" <<
    // getPanel(manager_panels.end()-1)->getName() << std::endl; std::cout <<
    // "add " << manager_panels.size() << std::endl;
}

// Получение указателя на панель по индексу (без передачи владения)
Panel *ManagerPanels::getPanel(size_t index) {
    // Проверка выхода за границы
    if (index >= getSize()) {
        return nullptr; // или бросить исключение std::out_of_range
    }
    return panels[index].get(); // возвращаем сырой указатель
}

// Удаление панели по индексу
void ManagerPanels::removePanel(size_t index) {
    if (index >= getSize()) {
        return; // или бросить исключение
    }
    panels.erase(panels.begin() + index);
}

// Удаление панели по указателю (если нужно)
bool ManagerPanels::removePanel(Panel *panel) {
    auto it =
        std::find_if(panels.begin(), panels.end(),
                     [panel](const auto &ptr) { return ptr.get() == panel; });

    if (it != panels.end()) {
        panels.erase(it);
        return true;
    }
    return false;
}
size_t ManagerPanels::getSize() { return panels.size(); }

// Удаление всех панелей
void ManagerPanels::removeAllPanels() { panels.clear(); }
