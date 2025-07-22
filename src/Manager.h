#pragma once
#include <vector>
#include <memory>
#include "Panel.h"

extern std::vector<std::unique_ptr<Panel>> manager_panels;  // Объявление

void addPanel(std::unique_ptr<Panel> panel);
Panel* getPanel(size_t index);
void removePanel(size_t index);
bool removePanel(Panel* panel);

