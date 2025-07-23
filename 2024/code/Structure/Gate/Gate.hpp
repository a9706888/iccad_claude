#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../Cell/Cell.hpp"

class Gate : public Cell {
   public:
    using ptr = std::unique_ptr<Gate>;

    Gate(const std::string &name, double width, double height, int pinCount)
        : Cell(name, width, height), pinCount(pinCount) {
        type = Cell::Type::GATE;
    }

   private:
    int pinCount;
};
