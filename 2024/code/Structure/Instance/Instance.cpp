#include "Instance.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <numeric>

Instance::Instance(const std::string &name, Coord loc, Cell *cell, size_t instID)
    : name(name),
      loc(loc),
      cell(cell),
      instID(instID),
      toBeDel(false),
      TNS(0.0) {
    if (cell->getType() != Cell::Type::IO) {
        input_pins.resize(cell->getInputPins().size());
        output_pins.resize(cell->getOutputPins().size());
        clk_pins.resize(1);
    }
}

void Instance::updtDlySlk(int idx) {
    auto instOut = output_pins[idx].get();
    auto instIn = input_pins[idx].get();
    // if (!instOut or !instIn) return;
    instOut->setDelay(getQpinDelay());
    updtBackDlySlk(instOut);
    updtFrontDelay(instIn);
}

void Instance::updtDlySlk() {
    size_t n = input_pins.size();
    for (size_t i = 0; i < n; i++) {
        updtDlySlk(i);
    }
}

void Instance::updtFrontDelay(Pin *instIn) {
    if (!instIn->getNet()) return;
    auto src = instIn->getNet()->getSrcPin();
    double netDelay = eval.dsplcDelay * src->Mdist(instIn);
    double delay = src->getDelay() + netDelay;
    instIn->setDelay(delay);
    instIn->setSlack(instIn->getRqrTime() - delay);
}

void Instance::updtBackDlySlk(Pin *start) {
    if (!start->getNet()) return;
    std::vector<Pin *> touched_pins;
    std::queue<Instance *> Q;
    for (auto &drain : start->getNet()->getDrainPins()) {
        if (drain->getInst()->isType(Cell::Type::FF)) {
            touched_pins.push_back(drain);
        } else if (drain->getInst()->isType(Cell::Type::GATE)) {
            Q.push(drain->getInst());
        }
    }
    while (!Q.empty()) {
        Instance *inst = Q.front();
        Q.pop();
        double maxDelay = 0;
        for (auto &p : inst->getInputPins()) {
            if (!p->getNet()) continue;
            auto src = p->getNet()->getSrcPin();
            double netDelay = eval.dsplcDelay * src->Mdist(p.get());
            maxDelay = std::max(maxDelay, src->getDelay() + netDelay);
        }
        if (maxDelay != inst->getOutputPin(0)->getDelay()) {
            for (auto &p : inst->getOutputPins()) {
                p->setDelay(maxDelay);
                if (!p->getNet()) continue;
                for (auto &drain : p->getNet()->getDrainPins()) {
                    if (drain->getInst()->isType(Cell::Type::FF)) {
                        touched_pins.push_back(drain);
                    } else if (drain->getInst()->isType(Cell::Type::GATE)) {
                        Q.push(drain->getInst());
                    }
                }
            }
        }
    }

    for (auto &in : touched_pins) {
        auto src = in->getNet()->getSrcPin();
        double netDelay = eval.dsplcDelay * src->Mdist(in);
        double delay = src->getDelay() + netDelay;
        in->setDelay(delay);
        in->setSlack(in->getRqrTime() - delay);
    }
}

void Instance::pinPermute() {
    size_t n = input_pins.size();
    if (n < 2) return;

    std::vector<Pin::ptr> originInputPins(n);
    std::vector<Pin::ptr> originOutputPins(n);
    for (size_t i = 0; i < n; i++) {
        // if (!input_pins[i]) continue;
        originInputPins[i] = moveInputPin(i);
        originOutputPins[i] = moveOutputPin(i);
    }

    double bestTNS = eval.TNS;
    std::vector<int> permutation(n);
    std::vector<int> bestPermutation;
    std::iota(permutation.begin(), permutation.end(), 0);
    bestPermutation = permutation;
    bool firstRound = true;
    do {
        if (firstRound) {
            firstRound = false;
            continue;
        }
        for (size_t i = 0; i < n; i++) {
            auto newIdx = permutation[i];
            // if (!originInputPins[newIdx]) continue;
            addInputPin(std::move(originInputPins[newIdx]), i);
            addOutputPin(std::move(originOutputPins[newIdx]), i);
            input_pins[i]->setID(i);
            output_pins[i]->setID(i);
        }
        updtDlySlk();

        if (eval.TNS > bestTNS) {
            bestTNS = eval.TNS;
            bestPermutation = permutation;
        }

        for (size_t i = 0; i < n; i++) {
            auto newIdx = permutation[i];
            // if (!input_pins[i]) continue;
            originInputPins[newIdx] = moveInputPin(i);
            originOutputPins[newIdx] = moveOutputPin(i);
        }
    } while (std::next_permutation(permutation.begin(), permutation.end()));

    for (size_t i = 0; i < n; i++) {
        auto newIdx = bestPermutation[i];
        // if (!originInputPins[newIdx]) continue;
        addInputPin(std::move(originInputPins[newIdx]), i);
        addOutputPin(std::move(originOutputPins[newIdx]), i);
        input_pins[i]->setID(i);
        output_pins[i]->setID(i);
    }

    updtDlySlk();
}

void Instance::swapLoc(Instance *other) {
    Coord tmp = loc;
    setLoc(other->loc);
    other->setLoc(tmp);
    updtDlySlk();
    other->updtDlySlk();
}

void Instance::swapPin(Instance *other, int pin_idx_self, int pin_idx_other) {
    auto tmp_input = moveInputPin(pin_idx_self);
    auto tmp_output = moveOutputPin(pin_idx_self);

    addInputPin(other->moveInputPin(pin_idx_other), pin_idx_self);
    addOutputPin(other->moveOutputPin(pin_idx_other), pin_idx_self);

    other->addInputPin(std::move(tmp_input), pin_idx_other);
    other->addOutputPin(std::move(tmp_output), pin_idx_other);
    updtDlySlk(pin_idx_self);
    other->updtDlySlk(pin_idx_other);
}

void Instance::addCLKNet(Net *net, size_t idx) {
    if (idx >= clk_pins.size()) {
        std::cerr << "idx out of range: try to assign " << idx
                  << " into clk_pins with size " << clk_pins.size() << "\n";
        clk_pins.resize(idx + 1);
    }
    assert(clk_pins.at(idx) != nullptr);
    clk_pins[idx]->setNet(net);
}

void Instance::addInputNet(Net *net, size_t pinID) {
    if (pinID >= input_pins.size()) {
        std::cerr << "pinID out of range: try to assign " << pinID
                  << " into input_pins with size " << input_pins.size() << "\n";
        input_pins.resize(pinID + 1);
    }
    assert(input_pins.at(pinID) != nullptr);
    input_pins.at(pinID)->setNet(net);
}

void Instance::addOutputNet(Net *net, size_t pinID) {
    if (pinID >= output_pins.size()) {
        std::cerr << "pinID out of range: try to assign " << pinID
                  << " into output_pins with size " << output_pins.size() << "\n";
        output_pins.resize(pinID + 1);
    }
    assert(output_pins.at(pinID) != nullptr);
    output_pins.at(pinID)->setNet(net);
}

void Instance::addCLKPin(Pin::ptr pin, size_t pinID) {
    pin->setID(pinID);
    pin->setLoc(static_cast<FlipFlop *>(cell)->getCLKpin()->getLoc() + loc);
    pin->setInst(this);
    if (pinID >= clk_pins.size()) {
        clk_pins.resize(pinID + 1);
    }
    clk_pins[pinID] = std::move(pin);
}

void Instance::addInputPin(Pin::ptr pin, size_t pinID) {
    pin->setID(pinID);
    if (cell->getInputPins().size() <= pinID) {
        std::cerr << "cell: " << cell->getName() << " input pin size: "
                  << cell->getInputPins().size() << " pinID: " << pinID << "\n";
    }
    assert(cell->getInputPins().size() > pinID);
    pin->setLoc(cell->getInputPin(pinID)->getLoc() + loc);
    pin->setInst(this);
    input_pins[pinID] = std::move(pin);
}

void Instance::addOutputPin(Pin::ptr pin, size_t pinID) {
    pin->setID(pinID);
    assert(cell->getOutputPins().size() > pinID);
    pin->setLoc(cell->getOutputPin(pinID)->getLoc() + loc);
    pin->setInst(this);
    output_pins[pinID] = std::move(pin);
}

void Instance::addNet2Pin(Net *net, const std::string &pin_name) {
    switch (std::toupper(pin_name[0])) {
        case 'D':  // FF input
            if (pin_name.size() == 1)
                addInputNet(net, 0);
            else
                addInputNet(net, std::stoi(pin_name.substr(1)));
            break;
        case 'Q':  // FF output
            if (pin_name.size() == 1)
                addOutputNet(net, 0);
            else
                addOutputNet(net, std::stoi(pin_name.substr(1)));
            break;
        case 'I':  // Input
            if (pin_name.size() == 2)
                addInputNet(net, 0);
            else
                addInputNet(net, std::stoi(pin_name.substr(2)) - 1);
            break;
        case 'O':  // Output
            if (pin_name.size() == 3)
                addOutputNet(net, 0);
            else
                addOutputNet(net, std::stoi(pin_name.substr(3)) - 1);
            break;
        case 'C':  // Clock
            addCLKNet(net, 0);
            break;
        default:
            break;
    }
}

void Instance::setLoc(Coord loc) {
    this->loc = loc;
    auto n = input_pins.size();
    for (size_t pinID = 0; pinID < n; pinID++) {
        // if (!input_pins[pinID]) continue;
        input_pins[pinID]->setLoc(
            cell->getInputPin(pinID)->getLoc() +
            this->loc);

        output_pins[pinID]->setLoc(
            cell->getOutputPin(pinID)->getLoc() +
            this->loc);
    }
    clk_pins[0]->setLoc(
        static_cast<FlipFlop *>(cell)->getCLKpin()->getLoc() +
        this->loc);
}

void Instance::setCell(Cell *cell) {
    eval.power += static_cast<FlipFlop *>(cell)->getPower() - getFF()->getPower();
    eval.area += cell->getArea() - getFF()->getArea();

    this->cell = cell;
    auto n = input_pins.size();
    for (size_t pinID = 0; pinID < n; pinID++) {
        input_pins[pinID]->setLoc(
            cell->getInputPin(pinID)->getLoc() +
            this->loc);

        output_pins[pinID]->setLoc(
            cell->getOutputPin(pinID)->getLoc() +
            this->loc);
    }
    clk_pins[0]->setLoc(
        static_cast<FlipFlop *>(cell)->getCLKpin()->getLoc() +
        this->loc);
}
