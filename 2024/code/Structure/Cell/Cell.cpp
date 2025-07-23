
#include "../Cell/Cell.hpp"

#include <algorithm>
#include <iostream>

#include "../../Solver/BoostInclude/BoostInclude.hpp"

Cell::Cell(const std::string &name, double width, double height)
    : name(name), width(width), height(height), area(width * height), type(Cell::Type::IO) {}

void Cell::printAllInputPins(bool isIOcell) {
    for (auto &pin : input_pins) {
        std::cout << (isIOcell ? "Input " : "Pin ")
                  << pin->getName() << " "
                  << pin->getLoc() << '\n';
    }
}

void Cell::printAllOutputPins(bool isIOcell) {
    for (auto &pin : output_pins) {
        std::cout << (isIOcell ? "Output " : "Pin ")
                  << pin->getName() << " "
                  << pin->getLoc() << '\n';
    }
}

// prevent unordered pinID (Ex: 3, 2, 0, 1)
void Cell::adjustPinID() {
    std::sort(input_pins.begin(), input_pins.end(),
              [](auto &a, auto &b) {
                  return a->getID() < b->getID();
              });

    std::sort(output_pins.begin(), output_pins.end(),
              [](auto &a, auto &b) {
                  return a->getID() < b->getID();
              });

    int pinID = 0;
    for (auto &pin : input_pins) {
        pin->setID(pinID++);
    }
    pinID = 0;
    for (auto &pin : output_pins) {
        pin->setID(pinID++);
    }
}
