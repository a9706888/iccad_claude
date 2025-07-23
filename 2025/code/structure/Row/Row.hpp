#ifndef ROW_HPP
#define ROW_HPP

#include <string>

class Row 
{
private:
    std::string name;
    std::string site;
    int x, y;
    std::string orientation;
    int numX, numY;
    int stepX, stepY;

public:
    // Constructor
    Row() : x(0), y(0), numX(0), numY(0), stepX(0), stepY(0) {}
    Row(const std::string& name, const std::string& site, int x, int y, 
        const std::string& orientation, int numX, int numY, int stepX, int stepY)
        : name(name), site(site), x(x), y(y), orientation(orientation), 
          numX(numX), numY(numY), stepX(stepX), stepY(stepY) {}

    // Getters
    const std::string& getName() const { return name; }
    const std::string& getSite() const { return site; }
    int getX() const { return x; }
    int getY() const { return y; }
    const std::string& getOrientation() const { return orientation; }
    int getNumX() const { return numX; }
    int getNumY() const { return numY; }
    int getStepX() const { return stepX; }
    int getStepY() const { return stepY; }

    // Setters
    void setName(const std::string& name) { this->name = name; }
    void setSite(const std::string& site) { this->site = site; }
    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }
    void setOrientation(const std::string& orientation) { this->orientation = orientation; }
    void setNumX(int numX) { this->numX = numX; }
    void setNumY(int numY) { this->numY = numY; }
    void setStepX(int stepX) { this->stepX = stepX; }
    void setStepY(int stepY) { this->stepY = stepY; }
    
    // Utility methods
    void setPosition(int x, int y) { this->x = x; this->y = y; }
    void setStep(int stepX, int stepY) { this->stepX = stepX; this->stepY = stepY; }
    void setNumber(int numX, int numY) { this->numX = numX; this->numY = numY; }
};

#endif // ROW_HPP