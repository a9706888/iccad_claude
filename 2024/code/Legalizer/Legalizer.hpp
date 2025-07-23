#pragma once
#include <vector>

class CellMgr;
class RowMgr;
class Coord;
class Instance;

class Legalizer {
   public:
    Legalizer(CellMgr* cellMgr, RowMgr* rowMgr);
    void legalize();
    std::vector<double> extractRowYs();
    std::vector<double> generateRowXs();
    void placeGateCells();
    void legalizeFFCells(const std::vector<double>& rowXs,
                         const std::vector<double>& rowYs);
    Coord findNearestSite(Instance* FF_inst,
                          const std::vector<double>& rowXs,
                          const std::vector<double>& rowYs,
                          std::vector<double>::const_iterator lower_x,
                          std::vector<double>::const_iterator lower_y,
                          int siteWidth,
                          int siteHeight);
    ~Legalizer() = default;

   private:
    CellMgr* cellMgr;
    RowMgr* rowMgr;
};
