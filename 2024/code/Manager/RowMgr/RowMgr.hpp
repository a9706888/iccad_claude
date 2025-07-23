#pragma once

#include <memory>
#include <unordered_set>
#include <vector>

#include "../../Solver/BoostInclude/BoostInclude.hpp"
#include "../../Structure/BitMatrix/BitMatrix.hpp"
#include "../../Structure/Coord/Coord.hpp"
#include "../../Structure/Row/Row.hpp"

class Legalizer;

class RowMgr {
   public:
    RowMgr() : Lbnd(DBL_MAX), Dbnd(DBL_MAX), Rbnd(-DBL_MAX), Ubnd(-DBL_MAX) {}

    void addRow(Row::ptr row);
    void initOccupiedSites();

    double getLbnd() const { return Lbnd; }
    double getRbnd() const { return Rbnd; }
    double getUbnd() const { return Ubnd; }
    double getDbnd() const { return Dbnd; }
    int getNumSites() const { return numSites; }
    int getSiteWidth() const { return siteWidth; }
    int getSiteHeight() const { return siteHeight; }
    BitMatrix& getOccupiedSites() { return occupiedSites; }
    const std::vector<Row::ptr>& getRows() const { return rows; }

   private:
    std::vector<Row::ptr> rows;
    BitMatrix occupiedSites;
    int siteWidth, siteHeight, numSites;
    double Lbnd, Dbnd, Rbnd, Ubnd;
};