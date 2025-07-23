#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../Solver/BoostInclude/BoostInclude.hpp"
#include "../Cell/Cell.hpp"
#include "../FlipFlop/FlipFlop.hpp"
#include "../Net/Net.hpp"
#include "../Pin/Pin.hpp"

class Net;
class Coord;
class CombCircuit;

class Instance {
   public:
    using ptr = std::unique_ptr<Instance>;

    Instance(const std::string &name, Coord loc, Cell *cell, size_t instID);

    void updtDlySlk(int idx);
    void updtDlySlk();
    void updtFrontDelay(Pin *instIn);
    void updtBackDlySlk(Pin *start);
    void pinPermute();

    void swapLoc(Instance *other);
    void swapPin(Instance *other, int pin_idx_self, int pin_idx_other);
    void addCLKNet(Net *net, size_t idx);
    void addInputNet(Net *net, size_t pinID);
    void addOutputNet(Net *net, size_t pinID);
    void addCLKPin(Pin::ptr pin, size_t pinID);
    void addInputPin(Pin::ptr pin, size_t pinID);
    void addOutputPin(Pin::ptr pin, size_t pinID);
    void addNet2Pin(Net *net, const std::string &pin_name);

    void setID(size_t id) { instID = id; }
    void setLoc(Coord _loc);
    void setCell(Cell *_cell);
    void setDeleted(bool _toBeDel) { toBeDel = _toBeDel; }

    const Coord &getLoc() const { return loc; }
    size_t getID() const { return instID; }
    Cell *getCell() const { return cell; }
    FlipFlop *getFF() const { return static_cast<FlipFlop *>(cell); }
    double getArea() const { return cell->getArea(); }
    Pin *getCLKPin(size_t idx) const { return clk_pins[idx].get(); }
    Pin *getInputPin(size_t idx) const { return input_pins[idx].get(); }
    Pin *getOutputPin(size_t idx) const { return output_pins[idx].get(); }
    const std::string &getName() const { return name; }
    const std::vector<Pin::ptr> &getCLKPins() const { return clk_pins; }
    const std::vector<Pin::ptr> &getInputPins() const { return input_pins; }
    const std::vector<Pin::ptr> &getOutputPins() const { return output_pins; }

    Coord getCenLoc() const {
        return loc + Coord(cell->getWidth() / 2, cell->getHeight() / 2);
    }
    double getQpinDelay() const {
        return getFF()->getQpinDelay();
    }

    Pin::ptr moveCLKPin(size_t idx) { return std::move(clk_pins[idx]); }
    Pin::ptr moveInputPin(size_t idx) { return std::move(input_pins[idx]); }
    Pin::ptr moveOutputPin(size_t idx) { return std::move(output_pins[idx]); }

    bool isDeleted() const { return toBeDel; }
    bool isType(Cell::Type type) const { return cell->getType() == type; }
    void resizeIOInstInputPins(int size) { input_pins.resize(size); }
    void resizeIOInstOutputPins(int size) { output_pins.resize(size); }

    bool sameShape(const Instance *other) const {
        return cell->sameShape(other->getCell());
    }
    double centerMdist(const Instance *other) const {
        return getCenLoc().Mdist(other->getCenLoc());
    }

    void calTNS() {
        TNS = 0;
        for (auto &p : input_pins) {
            auto slack = p->getSlack();
            if (slack < 0) TNS += p->getSlack();
        }
    }
    double getTNS() const { return TNS; }

   private:
    std::string name;
    Coord loc;
    Cell *cell;
    size_t instID;  // For FF: part of Name; for gate: position in topo order
    bool toBeDel;
    std::vector<Pin::ptr> input_pins;
    std::vector<Pin::ptr> output_pins;
    std::vector<Pin::ptr> clk_pins;  // First one is current used clk, the rest are old clks
    double TNS;
};
