#pragma once
#include <iostream>

#include "../Legalizer/Legalizer.hpp"
#include "../Manager/DataMgr/DataMgr.hpp"
#include "../Structure/CombCircuit/CombCircuit.hpp"
#include "../global/global.hpp"
#include "BoostInclude/BoostInclude.hpp"

#define SQRT2 sqrt((double)2)

class Solver {
   public:
    struct pair_hash {
        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2>& p) const {
            auto hash1 = std::hash<T1>{}(p.first);
            auto hash2 = std::hash<T2>{}(p.second);
            return hash1 ^ hash2;  // Combine the two hash values
        }
    };
    using TupleMapInt = std::unordered_map<std::pair<double, double>, int, pair_hash>;

    Solver(DataMgr* data);

    void preprocessing();
    void solve();
    void FFRelocate(Instance* FF_inst);
    void FFSwap(Instance* FF_inst);
    void checkOverlap();
    std::vector<FlipFlop*> buildBestFFEachSize();
    void fillDieWithBestFFs();
    void debanking();
    void banking();
    void findRestFFs(const Box& bestShell,
                     std::vector<Instance*>& merge_ffs,
                     const BoxRTree& FFinstBoxesOnly,
                     int shellSearchSide);
    Box findBestShell(const Instance* FF_inst, const FlipFlop* bestFitFF, double shellSearchSide);
    void pinSwap(Instance* FF_inst);
    bool outOfDie(const Box& box);
    Coord getBoxCen(const Box& box);
    std::pair<double, double> getboxShape(const Box& box);

    ~Solver() {};

   private:
    CellMgr* cellMgr;
    RowMgr* rowMgr;
    int siteWidth, siteHeight;
    BoxRTree allInstBoxes;
    BoxRTree FFShellBoxes;
    TupleMapInt shape2Size;
    // std::map<int, FlipFlop*> mtBit2FF;
    std::vector<FlipFlop*> bestFFOfEachSize;

    std::mt19937 generator{0};
    std::uniform_real_distribution<double> distribution{0, 1};
    std::uniform_int_distribution<int> distr1000{0, 1000};
};

inline bool Solver::outOfDie(const Box& box) {
    return box.min_corner.getX() < rowMgr->getLbnd() or
           box.min_corner.getY() < rowMgr->getDbnd() or
           box.max_corner.getX() > rowMgr->getRbnd() or
           box.max_corner.getY() > rowMgr->getUbnd();
}

inline Coord Solver::getBoxCen(const Box& box) {
    return Coord((box.min_corner.getX() + box.max_corner.getX()) / 2,
                 (box.min_corner.getY() + box.max_corner.getY()) / 2);
}

inline std::pair<double, double> Solver::getboxShape(const Box& box) {
    return {box.max_corner.getX() - box.min_corner.getX(),
            box.max_corner.getY() - box.min_corner.getY()};
}