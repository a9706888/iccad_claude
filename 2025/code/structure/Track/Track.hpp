#ifndef TRACK_HPP
#define TRACK_HPP

#include <string>

class Track 
{
private:
    std::string direction; // "X" or "Y"
    int start;
    int number;
    int step;
    std::string layer;

public:
    // Constructor
    Track() : start(0), number(0), step(0) {}
    Track(const std::string& direction, int start, int number, int step, const std::string& layer)
        : direction(direction), start(start), number(number), step(step), layer(layer) {}

    // Getters
    const std::string& getDirection() const { return direction; }
    int getStart() const { return start; }
    int getNumber() const { return number; }
    int getStep() const { return step; }
    const std::string& getLayer() const { return layer; }

    // Setters
    void setDirection(const std::string& direction) { this->direction = direction; }
    void setStart(int start) { this->start = start; }
    void setNumber(int number) { this->number = number; }
    void setStep(int step) { this->step = step; }
    void setLayer(const std::string& layer) { this->layer = layer; }
    
    // Utility methods
    bool isHorizontal() const { return direction == "X"; }
    bool isVertical() const { return direction == "Y"; }
    int getEndPosition() const { return start + (number - 1) * step; }
};

#endif // TRACK_HPP