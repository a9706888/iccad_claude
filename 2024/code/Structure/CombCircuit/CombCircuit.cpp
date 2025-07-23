#include "CombCircuit.hpp"

#include "../Net/Net.hpp"
#include "../Pin/Pin.hpp"

void CombCircuit::topoSort() {
    std::unordered_map<Instance*, int> inDegree;
    std::vector<Instance*> gatesInTopoOrder;
    std::queue<Instance*> Q;

    for (const auto& gate : gates) {
        int gateInDeg = 0;
        for (const auto& in : gate->getInputPins()) {
            if (!(in->getNet() and in->getNet()->getSrcPin())) {
                continue;
            }
            auto prevGate = in->getNet()->getSrcPin()->getInst();
            if (prevGate and prevGate->isType(Cell::Type::GATE)) {
                gateInDeg++;
            }
        }

        inDegree[gate] = gateInDeg;

        if (inDegree[gate] == 0) {
            Q.push(gate);
        }
    }
    int order = 0;
    while (!Q.empty()) {
        auto gate = Q.front();
        Q.pop();
        gate->setID(order++);
        gatesInTopoOrder.push_back(gate);

        for (auto& out : gate->getOutputPins()) {
            if (!out->getNet()) {
                continue;
            }
            for (auto pinOut : out->getNet()->getDrainPins()) {
                auto nextGate = pinOut->getInst();
                if (nextGate and nextGate->isType(Cell::Type::GATE)) {
                    inDegree[nextGate]--;
                    if (inDegree[nextGate] == 0) {
                        Q.push(nextGate);
                    }
                }
            }
        }
    }

    gates = gatesInTopoOrder;
}