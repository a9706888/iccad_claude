#include "Box.hpp"

#include "../Instance/Instance.hpp"

Box::Box(Instance* inst) {
    this->min_corner = inst->getLoc();
    this->max_corner = min_corner +
                       Coord(inst->getCell()->getWidth(),
                             inst->getCell()->getHeight());
    this->inst = inst;
}

Box::Box(Coord min_corner, Instance* inst) {
    this->min_corner = min_corner;
    this->max_corner = min_corner +
                       Coord(inst->getCell()->getWidth(),
                             inst->getCell()->getHeight());
    this->inst = inst;
}

Box::Box(Coord min_corner, Coord max_corner)
    : min_corner(min_corner), max_corner(max_corner) {}

Box::Box(Coord min_corner, Coord max_corner, Instance* inst)
    : Box(min_corner, max_corner) {
    this->inst = inst;
}