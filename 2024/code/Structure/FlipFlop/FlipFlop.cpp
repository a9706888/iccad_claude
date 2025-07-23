#include "FlipFlop.hpp"

#include "../../global/global.hpp"

FlipFlop::FlipFlop(const std::string &name, double width, double height, int pinCount, int bits)
    : Cell(name, width, height), pinCount(pinCount), bits(bits) {
    type = Cell::Type::FF;
    area = width * height;
    power = -1;
}
