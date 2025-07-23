#include "CellMgr.hpp"

#include "../../global/global.hpp"

CellMgr::CellMgr()
    : IO_Cell(std::make_unique<Cell>("IO-Cell", 0, 0)),
      IO_inst(std::make_unique<Instance>("IO_inst", Coord(0, 0), IO_Cell.get(), -1)) {
    FF_instances.reserve(22000);  // for # of FF in testcases < 22000
    gate_instances.reserve(1330000);
    FF_ID = 0;
}

void CellMgr::preprocess() {
    buildCombCirc();
    for (auto& combCircuit : combCircuits) {
        combCircuit->topoSort();
    }
    genRqrTime();

    calcInitScore();
    // genSlack();
}

void CellMgr::calcInitScore() {
    // TODO: the initial density score is not calculated here.
    for (auto& FF_inst : FF_instances) {
        auto FF = FF_inst->getFF();
        eval.area += FF->getArea();
        eval.power += FF->getPower();
    }
#ifdef DEBUG_MODE
    std::cout << "Initial score:\n";
    printCost();
#endif
}

Instance::ptr CellMgr::mergeFFs(FlipFlop* MBFF, const std::vector<Instance*>& toBeMergeFFs, Coord& lowerLeft) {
    std::string mergedFFNames;

    for (auto& FF_inst : toBeMergeFFs) {
        mergedFFNames += FF_inst->getName();
    }

    auto MBFF_inst = std::make_unique<Instance>(
        mergedFFNames,
        lowerLeft,
        MBFF,
        genInstID());

    int pinID = 0;
    for (auto& FF_inst : toBeMergeFFs) {
        for (size_t j = 0; j < FF_inst->getInputPins().size(); ++j, ++pinID) {
            MBFF_inst->addInputPin(FF_inst->moveInputPin(j), pinID);
            MBFF_inst->addOutputPin(FF_inst->moveOutputPin(j), pinID);
        }
    }
    pinID = 0;
    for (auto& FF_inst : toBeMergeFFs) {
        for (size_t j = 0; j < FF_inst->getCLKPins().size(); j++, pinID++) {
            MBFF_inst->addCLKPin(FF_inst->moveCLKPin(j), pinID);
        }
        FF_inst->setDeleted(true);
    }
    MBFF_inst->updtDlySlk();

    return MBFF_inst;
}

Instance::ptr CellMgr::splitFF(Instance* FF_inst, FlipFlop* smallerFF, Coord& loc, int idx, Pin* clkPin) {
    auto split_inst = std::make_unique<Instance>(
        FF_inst->getName() + "_split_" + std::to_string(idx),
        loc,
        smallerFF,
        genInstID());

    size_t nth_split = idx / smallerFF->getBits();
    size_t numClkPins = FF_inst->getCLKPins().size();
    if (nth_split < numClkPins) {
        split_inst->addCLKPin(FF_inst->moveCLKPin(nth_split), 0);
    } else {
        split_inst->addCLKPin(clkPin->duplicate(), 0);
    }

    if (idx + smallerFF->getBits() == FF_inst->getFF()->getBits()) {
        if (nth_split + 1 < numClkPins) {
            for (size_t i = nth_split + 1, j = 1; i < numClkPins; i++, j++) {
                split_inst->addCLKPin(FF_inst->moveCLKPin(i), j);
            }
        }
    }

    size_t n = smallerFF->getBits();

    for (size_t i = 0; i < n; i++) {
        split_inst->addInputPin(FF_inst->moveInputPin(idx + i), i);
        split_inst->addOutputPin(FF_inst->moveOutputPin(idx + i), i);
    }

    split_inst->updtDlySlk();

    return split_inst;
}

void CellMgr::buildCombCirc() {
    int gate_num = 0;
    std::unordered_set<Instance*> visited_gates;
    for (auto& G_inst : gate_instances) {
        if (visited_gates.count(G_inst.get())) continue;

        auto combCircuit = std::make_unique<CombCircuit>();

        std::vector<Instance*> connected_gates;
        connected_gates.push_back(G_inst.get());
        while (!connected_gates.empty()) {
            auto connected_gate = connected_gates.back();
            connected_gates.pop_back();

            if (visited_gates.count(connected_gate)) continue;
            visited_gates.insert(connected_gate);
            combCircuit->addGate(connected_gate);
            gate_num++;

            for (auto& in : connected_gate->getInputPins()) {
                if (!(in->getNet() and in->getNet()->getSrcPin())) continue;
                auto prevGate = in->getNet()->getSrcPin()->getInst();
                if (prevGate and prevGate->isType(Cell::Type::GATE)) {
                    connected_gates.push_back(prevGate);
                }
            }
            for (auto& out : connected_gate->getOutputPins()) {
                if (!out->getNet()) continue;
                for (auto& drain : out->getNet()->getDrainPins()) {
                    auto nextGate = drain->getInst();
                    if (nextGate and nextGate->isType(Cell::Type::GATE)) {
                        connected_gates.push_back(nextGate);
                    }
                }
            }
        }
        if (combCircuit->getGates().size() > 0) {
            combCircuits.emplace_back(std::move(combCircuit));
        }
    }
}

void CellMgr::genDelay() {
    for (auto& G_inst : gate_instances) {
        for (auto& p : G_inst->getInputPins()) {
            p->setDelay(0);
        }
        for (auto& p : G_inst->getOutputPins()) {
            p->setDelay(0);
        }
    }
    for (auto& pi : IO_inst->getInputPins()) {
        pi->setDelay(0);
    }
    for (auto& FF_inst : FF_instances) {
        if (FF_inst->isDeleted()) continue;
        for (auto& p : FF_inst->getInputPins()) {
            p->setDelay(0);
        }
        for (auto& p : FF_inst->getOutputPins()) {
            p->setDelay(FF_inst->getQpinDelay());
        }
    }
    for (auto& combCircuit : combCircuits) {
        for (Instance* gate : combCircuit->getGates()) {
            double maxDelay = 0;
            for (auto& in : gate->getInputPins()) {
                if (!(in->getNet() and in->getNet()->getSrcPin())) continue;
                auto src = in->getNet()->getSrcPin();
                double netDelay = eval.dsplcDelay * src->Mdist(in.get());
                double delay = src->getDelay() + netDelay;
                in->setDelay(delay);
                maxDelay = std::max(maxDelay, delay);
            }
            for (auto& p : gate->getOutputPins()) {
                p->setDelay(maxDelay);
            }
        }
    }
    for (auto& FF_inst : FF_instances) {
        if (FF_inst->isDeleted()) continue;
        for (auto& in : FF_inst->getInputPins()) {
            auto src = in->getNet()->getSrcPin();
            double netDelay = eval.dsplcDelay * src->Mdist(in.get());
            double delay = src->getDelay() + netDelay;
            in->setDelay(delay);
        }
    }
}

void CellMgr::genRqrTime() {
    genDelay();
    for (auto& FF_inst : FF_instances) {
        for (auto& p : FF_inst->getInputPins()) {
            p->setRqrTime(p->getDelay() + p->getSlack());
        }
    }
}

void CellMgr::genSlack() {
    genDelay();
    for (auto& FF_inst : FF_instances) {
        if (FF_inst->isDeleted()) continue;
        for (auto& p : FF_inst->getInputPins()) {
            p->setSlack(p->getRqrTime() - p->getDelay());
        }
    }
}

void CellMgr::rmvAndDltInst() {
    auto removeIt = std::remove_if(
        FF_instances.begin(), FF_instances.end(), [&](auto& inst) {
            if (inst->isDeleted()) {
                eval.area -= inst->getFF()->getArea();
                eval.power -= inst->getFF()->getPower();
                return true;
            }
            return false;
        });

    FF_instances.erase(removeIt, FF_instances.end());
}

Coord CellMgr::relocate(Instance* FF_inst, Coord::CoordSet& jumpLocs) {
    if (jumpLocs.empty()) {
        return FF_inst->getLoc();
    }

    double bestScore = eval.getScore();
    Coord bestLoc = FF_inst->getLoc();

    for (auto& jumploc : jumpLocs) {
        eval.shiftInst(FF_inst, jumploc);

        if (bestScore > eval.getScore()) {
            bestScore = eval.getScore();
            bestLoc = jumploc;
        }
    }
    eval.shiftInst(FF_inst, bestLoc);

    return bestLoc;
}

double CellMgr::getSwapTNS(Instance* inst_A, Instance* inst_B) {
    double bestTNS = -eval.TNS;

    inst_A->swapLoc(inst_B);

    double newTNS = -eval.TNS;

    inst_A->swapLoc(inst_B);

    if (newTNS > bestTNS) {
        return DBL_MAX;
    } else {
        return newTNS;
    }
}

double CellMgr::getSwapTNS(Instance* inst_A, Instance* inst_B, int pin_idx_A, int pin_idx_B) {
    double bestTNS = -eval.TNS;

    inst_A->swapPin(inst_B, pin_idx_A, pin_idx_B);

    double newTNS = -eval.TNS;

    inst_A->swapPin(inst_B, pin_idx_A, pin_idx_B);

    if (newTNS > bestTNS) {
        return DBL_MAX;
    } else {
        return newTNS;
    }
}

void CellMgr::addNet(Net::ptr net) {
    if (net->isCLKNet()) {
        clk_nets.emplace_back(move(net));
    } else {
        nets.emplace_back(move(net));
    }
}

void CellMgr::adjustPinID(bool isFF) {
    if (isFF) {
        FF_lib.back()->adjustPinID();
    } else {
        Gate_lib.back()->adjustPinID();
    }
}

void CellMgr::addFF2Lib(FlipFlop::ptr FF, CellMap& cellMap) {
    cellMap[FF->getName()] = FF.get();
    FF_lib.emplace_back(move(FF));
}

void CellMgr::addGate2Lib(Gate::ptr gate, CellMap& cellMap) {
    cellMap[gate->getName()] = gate.get();
    Gate_lib.emplace_back(move(gate));
}

void CellMgr::addInst(InstMap& instMap, InstPinMap& instPinMap, Instance::ptr inst) {
    auto raw_inst = inst.get();
    Cell* cell = inst->getCell();
    if (cell->getType() == Cell::Type::FF) {
        FlipFlop* flipflop = static_cast<FlipFlop*>(cell);
        size_t tag = old_tags.size();
        int n = flipflop->getBits();
        int numOldNodes = (2 * n) + 1;
        old_tags.resize(old_tags.size() + numOldNodes, {"", ""});  // allocate 2n+1 empty node (inst_name, pin_name)

        instMap[raw_inst->getName()] = raw_inst;
        FF_instances.emplace_back(std::move(inst));

        for (auto& pin : cell->getInputPins()) {
            raw_inst->addInputPin(
                std::make_unique<Pin>(
                    pin->getName(), tag, raw_inst,
                    pin->getLoc() + raw_inst->getLoc(),
                    Pin::Type::D),
                pin->getID());

            instPinMap[raw_inst][pin->getName()] = raw_inst->getInputPin(pin->getID());
            old_tags[tag] = {raw_inst->getName(), pin->getName()};
            tag += 2;
        }

        {
            auto clkID = flipflop->getCLKpin()->getID();
            std::string clkName = flipflop->getCLKpin()->getName();
            raw_inst->addCLKPin(
                std::make_unique<Pin>(
                    clkName, tag, raw_inst,
                    flipflop->getCLKpin()->getLoc() + raw_inst->getLoc(),
                    Pin::Type::CLK),
                clkID);

            instPinMap[raw_inst][clkName] = raw_inst->getCLKPin(clkID);
            old_tags[tag] = {raw_inst->getName(), clkName};  // clock pin
        }

        tag = old_tags.size() - numOldNodes + 1;
        for (auto& pin : cell->getOutputPins()) {
            raw_inst->addOutputPin(
                std::make_unique<Pin>(
                    pin->getName(), tag, raw_inst,
                    pin->getLoc() + raw_inst->getLoc(),
                    Pin::Type::Q),
                pin->getID());

            instPinMap[raw_inst][pin->getName()] = raw_inst->getOutputPin(pin->getID());
            old_tags[tag] = {raw_inst->getName(), pin->getName()};
            tag += 2;
        }

    } else {
        instMap[raw_inst->getName()] = raw_inst;
        gate_instances.emplace_back(std::move(inst));

        for (auto& pin : cell->getInputPins()) {
            raw_inst->addInputPin(
                std::make_unique<Pin>(
                    pin->getName(), raw_inst,
                    pin->getLoc() + raw_inst->getLoc(),
                    Pin::Type::IN),
                pin->getID());

            instPinMap[raw_inst][pin->getName()] = raw_inst->getInputPin(pin->getID());
        }
        for (auto& pin : cell->getOutputPins()) {
            raw_inst->addOutputPin(
                std::make_unique<Pin>(
                    pin->getName(), raw_inst,
                    pin->getLoc() + raw_inst->getLoc(),
                    Pin::Type::OUT),
                pin->getID());

            instPinMap[raw_inst][pin->getName()] = raw_inst->getOutputPin(pin->getID());
        }
    }
}

void CellMgr::addPin2Cell(Pin::ptr pin) {
    Cell* cell;
    switch (pin->getType()) {
        case Pin::Type::D: {
            cell = FF_lib.back().get();
            cell->input_pins.emplace_back(std::move(pin));
            break;
        }
        case Pin::Type::Q: {
            cell = FF_lib.back().get();
            cell->output_pins.emplace_back(std::move(pin));
            break;
        }
        case Pin::Type::IN: {
            cell = Gate_lib.back().get();
            cell->input_pins.emplace_back(std::move(pin));
            break;
        }
        case Pin::Type::OUT: {
            cell = Gate_lib.back().get();
            cell->output_pins.emplace_back(std::move(pin));
            break;
        }
        case Pin::Type::CLK: {
            cell = FF_lib.back().get();
            FlipFlop* flipflop = static_cast<FlipFlop*>(cell);
            flipflop->setCLKpin(std::move(pin));
            break;
        }
        case Pin::Type::INPUT: {
            cell = IO_Cell.get();
            cell->input_pins.emplace_back(std::move(pin));
            break;
        }
        case Pin::Type::OUTPUT: {
            cell = IO_Cell.get();
            cell->output_pins.emplace_back(std::move(pin));
            break;
        }
        default: {
            std::cerr << "Error: " << "Unknown pin type\n";
            break;
        }
    }
}

void CellMgr::setTimingSlack(Instance* inst, str pin_name, double slack) {
    for (auto& pin : inst->getInputPins()) {
        if (pin->getName() == pin_name) {
            pin->setSlack(slack);
            return;
        }
    }
    std::cerr << "Error: " << pin_name << " not found in " << inst->getName() << "\n";
    return;
}

void CellMgr::setQpinDelay(const str& cell_name, double delay, Cell* cell) {
    if (cell->getType() == Cell::Type::FF) {
        FlipFlop* flipflop = static_cast<FlipFlop*>(cell);
        flipflop->setQpinDelay(delay);
    } else {
        std::cerr << "Error: " << cell_name << " is not a FlipFlop\n";
    }
}

void CellMgr::setPower(Cell* cell, double power) {
    if (cell->getType() == Cell::Type::FF) {
        FlipFlop* flipflop = static_cast<FlipFlop*>(cell);
        flipflop->setPower(power);
    } else {
        std::cerr << "Error: " << cell->getName() << " is not a FlipFlop\n";
    }
}

const std::vector<Instance::ptr>& CellMgr::getInstByType(Cell::Type type) {
    if (type == Cell::Type::FF) {
        return FF_instances;
    } else if (type == Cell::Type::GATE) {
        return gate_instances;
    } else {
        std::cerr << "Error: " << "Unknown type\n";
        exit(1);
    }
}

void CellMgr::printCost() const {
    std::cout << std::setprecision(10)
              << "Weighted cost = "
              << eval.getTnsScore() << "(TNS) + "
              << eval.getPowerScore() + eval.getAreaScore() << "(PwrArea) = "
              << eval.getScore() << "\n\n";
}

void CellMgr::printFFcnt() const {
    std::map<int, int> FFCountByBits;
    for (auto& FF_inst : FF_instances) {
        int bits = std::count_if(FF_inst->getInputPins().begin(), FF_inst->getInputPins().end(), [](const auto& pin) { return pin != nullptr; });
        FFCountByBits[bits]++;
    }

    std::cout << "FF count by bits:\n";
    for (const auto& [bits, count] : FFCountByBits) {
        std::cout << bits << " bits: " << count << "\n";
    }
    std::cout << "Total FF count: " << FF_instances.size() << "\n";
}

void CellMgr::printLowerBound(std::vector<FlipFlop*>& bestFFOfEachSize) const {
    double best4_score = bestFFOfEachSize[0]->pwrAreaScore();
    double best2_score = bestFFOfEachSize[1]->pwrAreaScore();
    double best1_score = bestFFOfEachSize[2]->pwrAreaScore();

    double lowerBound = 0;
    for (const auto& clk_net : clk_nets) {
        int bits_cnt = 0;
        for (const auto& pin : clk_net->getclkNetPins()) {
            auto inst = pin->getInst();
            bits_cnt += inst->getFF()->getBits();
        }
        lowerBound += (bits_cnt / 4) * best4_score +
                      ((bits_cnt % 4) / 2) * best2_score +
                      ((bits_cnt % 2) / 1) * best1_score;
    }
    std::cout << "Score Lower bound = " << lowerBound << "\n\n";
}