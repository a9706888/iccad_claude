#pragma once
#include <iostream>

#include "../Coord/Coord.hpp"


//eval.die->initialize(Coord(lx, ly), width, height);
class Die {
   public:
    Die() = default;

    void initialize(Coord loc, double width, double height) {
        this->loc = loc;
        this->width = width;
        this->height = height;
    }

    const Coord &getLoc() const { return loc; }

    Coord loc;  // lower left coordinate of die
    double width, height;
};
