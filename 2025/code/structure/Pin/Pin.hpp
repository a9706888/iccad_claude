#ifndef PIN_HPP
#define PIN_HPP

#include <string>

class Pin 
{
private:
    std::string name;
    std::string net;
    std::string direction;
    int x, y;

public:
    // Constructor
    Pin() : x(0), y(0) {}
    Pin(const std::string& name, const std::string& net, const std::string& direction, int x, int y)
        : name(name), net(net), direction(direction), x(x), y(y) {}

    // Getters
    const std::string& getName() const { return name; }
    const std::string& getNet() const { return net; }
    const std::string& getDirection() const { return direction; }
    int getX() const { return x; }
    int getY() const { return y; }

    // Setters
    void setName(const std::string& name) { this->name = name; }
    void setNet(const std::string& net) { this->net = net; }
    void setDirection(const std::string& direction) { this->direction = direction; }
    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }
    
    // Utility methods
    void setPosition(int x, int y) { this->x = x; this->y = y; }
};

#endif // PIN_HPP