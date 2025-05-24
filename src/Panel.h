#pragma once
#include <string>
#include <imgui.h>

class Panel {
public:
    Panel(const std::string& name) : name(name), isOpen(true) {}
    virtual ~Panel() = default;

    virtual void render() = 0;  // ���� ����㠫�� ��⮤
    const std::string& getName() const { return name; }
    bool& getIsOpen() { return isOpen; }

protected:
    std::string name;
    bool isOpen;
};

