#pragma once
#include "Panel.h"
#include <cstddef>
#include <memory>
#include <vector>

class ManagerPanels {

    public:
        void addPanel(std::unique_ptr<Panel> panel);
        Panel *getPanel(size_t index);
        void removePanel(size_t index);
        bool removePanel(Panel *panel);
        void removeAllPanels();
        size_t getSize();
        void setNextPanel(bool n);
        bool &getNextEnd() { return NextEnd; }

    protected:
        std::vector<std::unique_ptr<Panel>> panels;

        bool NextEnd = false; // если надо прыгнуть на конкретный индекс панели
};

inline ManagerPanels manager_panels;
