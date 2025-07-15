#pragma once
#include <imgui.h>
#include <string>

#include <memory>
#include <vector>


class Panel {
    public:
        Panel(const std::string &name)
            : name(name),
              isOpen(true) {}
        virtual ~Panel() = default;

        virtual void render() = 0; // ололол
        const std::string &getName() const { return name; }
        bool &getIsOpen() { return isOpen; }

    protected:
        std::string name;
        bool isOpen;
};


// void render_all_panels();
// bool push_panel(Panel *pnl);
// bool remove_panel();

static std::vector<std::unique_ptr<Panel>> manager_panels;

