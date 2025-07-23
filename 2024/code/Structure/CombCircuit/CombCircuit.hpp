#pragma once

#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../Instance/Instance.hpp"

class CombCircuit {
    std::vector<Instance*> gates;

   public:
    using ptr = std::unique_ptr<CombCircuit>;
    CombCircuit() = default;

    void topoSort();

    void addGate(Instance* inst){
        gates.push_back(inst);
    }
    const std::vector<Instance*>& getGates() const{
        return gates;
    }
    Instance* getGate(int i) const{
        return gates[i];
    };
};