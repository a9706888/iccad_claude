#pragma once

#include <limits>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../../Structure/Cell/Cell.hpp"
#include "../../Structure/CombCircuit/CombCircuit.hpp"
#include "../../Structure/FlipFlop/FlipFlop.hpp"
#include "../../Structure/Gate/Gate.hpp"
#include "../../Structure/Instance/Instance.hpp"
#include "../../Structure/Net/Net.hpp"

class CellMgr {
   public:
    using ptr = std::unique_ptr<CellMgr>;
    using str = std::string;
    using CellMap = std::unordered_map<str, Cell *>;
    using InstMap = std::unordered_map<str, Instance *>;
    using InstPinMap = std::unordered_map<Instance *, std::unordered_map<str, Pin *>>;

    CellMgr();
    size_t genInstID();
    void preprocess();
    void calcInitScore();
    Instance::ptr mergeFFs(FlipFlop *MBFF, const std::vector<Instance *> &toBeMergeFFs, Coord &lowerLeft);
    Instance::ptr splitFF(Instance *FF_inst, FlipFlop *smallerFF, Coord &loc, int idx, Pin *clkPin);
    void buildCombCirc();
    void genDelay();
    void genRqrTime();
    void genSlack();
    void rmvAndDltInst();
    Coord relocate(Instance *FF_inst, Coord::CoordSet &jumpLocs);
    double getSwapTNS(Instance *inst_A, Instance *inst_B);
    double getSwapTNS(Instance *inst_A, Instance *inst_B, int pin_idx_A, int pin_idx_B);

    void addNet(Net::ptr net);
    void adjustPinID(bool isFF);
    void addFF2Lib(FlipFlop::ptr FF, CellMap &cellMap);
    void addGate2Lib(Gate::ptr gate, CellMap &cellMap);
    void addInst(InstMap &instMap, InstPinMap &instPinMap, Instance::ptr inst);
    void addCreatedInst(Instance::ptr inst);
    void addPin2Cell(Pin::ptr pin);

    void setTimingSlack(Instance *inst, str pin_name, double slack);
    void setQpinDelay(const str &cell_name, double delay, Cell *cell);
    void setPower(Cell *cell, double power);

    Cell *getIOCell() { return IO_Cell.get(); }
    Instance *getIOInst() { return IO_inst.get(); }

    const std::vector<Net::ptr> &getCLKNets() { return clk_nets; }
    const std::vector<FlipFlop::ptr> &getFFLibCells() { return FF_lib; }
    std::pair<str, str> &getOldNode(size_t idx) { return old_tags[idx]; }
    const std::vector<Instance::ptr> &getInstByType(Cell::Type type);

    void printCost() const;
    void printFFcnt() const;
    void printLowerBound(std::vector<FlipFlop *> &bestFFOfEachSize) const;

    friend class Parser;

   private:
    Cell::ptr IO_Cell;
    std::vector<FlipFlop::ptr> FF_lib;
    std::vector<Gate::ptr> Gate_lib;

    Instance::ptr IO_inst;
    std::vector<Instance::ptr> FF_instances;
    std::vector<Instance::ptr> gate_instances;
    int FF_ID;

    FlipFlop *oneBitFF;
    std::vector<std::pair<str, str>> old_tags;  // store original design: (Inst_name, pin_name) pairs
    std::vector<CombCircuit::ptr> combCircuits;

    std::vector<Net::ptr> nets;  // non-CLK net
    std::vector<Net::ptr> clk_nets;

    std::mt19937 generator{0};
    std::uniform_real_distribution<double> distribution{0, 1};
};

inline void CellMgr::addCreatedInst(Instance::ptr inst) {
    eval.area += inst->getFF()->getArea();
    eval.power += inst->getFF()->getPower();
    FF_instances.emplace_back(std::move(inst));
}

inline size_t CellMgr::genInstID() { return FF_ID++; }