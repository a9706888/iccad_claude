#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <string>

class Component 
{
private:
    std::string name;
    std::string macro;
    int x, y;
    std::string orient;

public:
    // Constructor
    Component() : x(0), y(0) {}
    Component(const std::string& name, const std::string& macro, int x, int y, const std::string& orient)
        : name(name), macro(macro), x(x), y(y), orient(orient) {}

    // Getters
    const std::string& getName() const { return name; }
    const std::string& getMacro() const { return macro; }
    int getX() const { return x; }
    int getY() const { return y; }
    const std::string& getOrientation() const { return orient; }

    // Setters
    void setName(const std::string& name) { this->name = name; }
    void setMacro(const std::string& macro) { this->macro = macro; }
    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }
    void setOrientation(const std::string& orient) { this->orient = orient; }
    
    // Utility methods
    void setPosition(int x, int y) { this->x = x; this->y = y; }
};

#endif // COMPONENT_HPP