#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../Pin/Pin.hpp"

class Cell {
   public:
    using ptr = std::unique_ptr<Cell>;
    enum Type {
        IO,
        FF,
        GATE,
        UNKNOWN,
    };

    Cell(const std::string &name, double width, double height);

    void printAllInputPins(bool isIOcell);
    void printAllOutputPins(bool isIOcell);
    void adjustPinID();
    bool sameShape(Cell *other) const {
        return width == other->getWidth() and height == other->getHeight();
    }

    int getType() const { return type; }
    double getArea() const { return area; }
    double getWidth() const { return width; }
    double getHeight() const { return height; }
    const std::string &getName() const { return name; }
    Pin *getInputPin(size_t idx) const { return input_pins[idx].get(); }
    Pin *getOutputPin(size_t idx) const { return output_pins[idx].get(); }
    const std::vector<Pin::ptr> &getInputPins() const { return input_pins; }
    const std::vector<Pin::ptr> &getOutputPins() const { return output_pins; }

    friend class CellMgr;

   protected:
    const std::string name;
    double width, height;
    double area;
    int type;

    std::vector<Pin::ptr> input_pins;
    std::vector<Pin::ptr> output_pins;
};