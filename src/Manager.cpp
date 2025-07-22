
#include "Manager.h"
#include "Panel.h"

#include <algorithm> // для std::find_if

#include <iostream>


// Определение вектора
std::vector<std::unique_ptr<Panel>> manager_panels;

void addPanel(std::unique_ptr<Panel> panel) {
    manager_panels.push_back(std::move(panel));
    // std::cout << "add " << manager_panels.size() << " Panel:" << getPanel(manager_panels.end()-1)->getName() << std::endl;
    // std::cout << "add " << manager_panels.size() << std::endl;
}


// Получение указателя на панель по индексу (без передачи владения)
Panel* getPanel(size_t index) {
    // Проверка выхода за границы
    if (index >= manager_panels.size()) {
        return nullptr; // или бросить исключение std::out_of_range
    }
    return manager_panels[index].get(); // возвращаем сырой указатель
}

// Удаление панели по индексу
void removePanel(size_t index) {
    if (index >= manager_panels.size()) {
        return; // или бросить исключение
    }
    manager_panels.erase(manager_panels.begin() + index);
}

// Удаление панели по указателю (если нужно)
bool removePanel(Panel* panel) {
    auto it = std::find_if(manager_panels.begin(), manager_panels.end(),
        [panel](const auto& ptr) { return ptr.get() == panel; });
    
    if (it != manager_panels.end()) {
        manager_panels.erase(it);
        return true;
    }
    return false;
}
