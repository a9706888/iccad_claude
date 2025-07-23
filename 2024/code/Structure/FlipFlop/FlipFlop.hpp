#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../../global/global.hpp"
#include "../Cell/Cell.hpp"

class Pin;

class FlipFlop : public Cell {
   public:
    using ptr = std::unique_ptr<FlipFlop>;
    FlipFlop(const std::string &name, double width, double height, int pinCount, int bits);

    double pwrAreaScore() const { return eval.beta * power + eval.gamma * area; }
    double calcScore() const {
        return (pwrAreaScore() / bits) + QpinDelay;
    }
    bool isBetterThan(FlipFlop *old_FF) const {
        return (old_FF == nullptr) ? true : calcScore() < old_FF->calcScore();
    }

    int getBits() const { return bits; }
    Pin *getCLKpin() const { return clkPin.get(); }
    double getArea() const { return area; }
    double getPower() const { return power; }
    double getQpinDelay() const { return QpinDelay; }

    void setPower(double power) { this->power = power; }
    void setCLKpin(Pin::ptr clkPin) { this->clkPin = std::move(clkPin); }
    void setQpinDelay(double QpinDelay) { this->QpinDelay = QpinDelay; }

   private:
    int pinCount;
    int bits;
    double power;
    double QpinDelay;  // clock-to-Q propagation delay
    Pin::ptr clkPin;
};
