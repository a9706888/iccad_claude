#pragma once
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include "../Structure/Die/Die.hpp"

class CellMgr;
class Instance;
class Evaluator {
   public:
    Evaluator()
        : TNS(0),
          area(0),
          power(0),
          violatedBins(0),
          die(std::make_unique<Die>()) {}

    void initialize(double a, double b, double g, double lmb);
    void setBin(double bw, double bh, double bmu);

    double getTnsScore() const { return alpha * -TNS; }
    double getPowerScore() const { return beta * power; }
    double getAreaScore() const { return gamma * area; }
    double getDensityScore() const { return lambda * violatedBins; }
    double getScore() const;

    void shiftInst(Instance* inst, const Coord& newLoc);
    double initDensity(CellMgr* cellMgr);

    void printEvaluationInfo();

    double alpha, beta, gamma, lambda, dsplcDelay;
    double TNS, area, power;
    int violatedBins;
    double binWidth, binHeight, binMaxUtil;
    std::unique_ptr<Die> die;
    std::vector<std::vector<double>> binAreas;

   private:
    void add2Bin(Instance* inst);
    std::vector<std::tuple<int, int, double>> computeBinContrib(Instance* inst,
                                                                const Coord& loc,
                                                                int numBinsX,
                                                                int numBinsY) const;
    inline double getAllowedArea(int i, int j, int numBinsX, int numBinsY) const;
};
