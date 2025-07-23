#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../Pin/Pin.hpp"

class Net {
   public:
    using str = std::string;
    using ptr = std::unique_ptr<Net>;

    Net(const std::string &name) : name(name), CLKcnt(0), src_pin(nullptr) {}

    void addPin2Net(Pin *pin);
    Coord getFFMidPoint() const;
    Pin *getSrcPin() const { return src_pin; }
    bool isCLKNet() const { return CLKcnt > 0; }
    const std::string &getName() const { return name; }
    const std::vector<Pin *> &getDrainPins() const { return drain_pins; }
    const std::vector<Pin *> &getclkNetPins() const { return clk_pins; }

   private:
    std::string name;
    int CLKcnt;
    Pin *src_pin;                   // could be Q, INPUT, OUT: source signal of net
    std::vector<Pin *> drain_pins;  // could be D, OUTPUT, IN: sink signal of net
    std::vector<Pin *> clk_pins;    // includes CLK only
};

inline void Net::addPin2Net(Pin *pin) {
    if (pin->getType() == Pin::Type::CLK) {
        clk_pins.push_back(pin);
        CLKcnt++;
    } else if (pin->getType() == Pin::Type::Q or pin->getType() == Pin::Type::OUT or pin->getType() == Pin::Type::INPUT) {
        src_pin = pin;
    } else {
        drain_pins.push_back(pin);
    }
}

inline Coord Net::getFFMidPoint() const {
    double x = 0, y = 0;
    for (auto &pin : clk_pins) {
        if (pin->getType() == Pin::Type::CLK) {
            x += pin->getLoc().getX();
            y += pin->getLoc().getY();
        }
    }
    x /= clk_pins.size();
    y /= clk_pins.size();
    return Coord(x, y);
}
