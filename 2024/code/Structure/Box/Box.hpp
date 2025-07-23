#pragma once
#include "../Coord/Coord.hpp"

class Instance;

class Box {
public:
    Coord min_corner, max_corner;
    Instance* inst;
    
    Box() = default;
    Box(Instance* inst);
    Box(Coord min_corner, Instance* inst);
    Box(Coord min_corner, Coord max_corner);
    Box(Coord min_corner, Coord max_corner, Instance* inst);
};