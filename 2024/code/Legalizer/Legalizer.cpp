#include "Legalizer.hpp"

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <iostream>

#include "../Manager/CellMgr/CellMgr.hpp"
#include "../Manager/RowMgr/RowMgr.hpp"
#include "../Structure/FlipFlop/FlipFlop.hpp"
#include "../global/global.hpp"

Legalizer::Legalizer(CellMgr* cellMgr, RowMgr* rowMgr)
    : cellMgr(cellMgr), rowMgr(rowMgr) {}

std::vector<double> Legalizer::extractRowYs() {
    std::vector<double> rowYs;
    for (auto& row : rowMgr->getRows()) {
        rowYs.push_back(row->getLoc().getY());
    }
    std::sort(rowYs.begin(), rowYs.end());
    return rowYs;
}

void Legalizer::legalize() {
#ifdef DEBUG_MODE
    timer.startTimer("legalize");
#endif

    auto rowYs = extractRowYs();
    auto rowXs = generateRowXs();
    rowMgr->initOccupiedSites();

    placeGateCells();
    legalizeFFCells(rowXs, rowYs);

    cellMgr->genSlack();
#ifdef DEBUG_MODE
    timer.stopTimer("legalize");
#endif
}

std::vector<double> Legalizer::generateRowXs() {
    std::vector<double> rowXs;
    int siteWidth = rowMgr->getSiteWidth();
    int numSites = rowMgr->getNumSites();
    double minX = rowMgr->getRows()[0]->getLoc().getX();
    double maxX = minX + siteWidth * numSites;
    for (double x = minX; x < maxX; x += siteWidth) {
        rowXs.push_back(x);
    }
    return rowXs;
}

void Legalizer::placeGateCells() {
    auto& occupiedSite = rowMgr->getOccupiedSites();
    for (auto& gateInst : cellMgr->getInstByType(Cell::Type::GATE)) {
        occupiedSite.place(gateInst->getCell(), gateInst->getLoc());
    }
}

void Legalizer::legalizeFFCells(const std::vector<double>& rowXs,
                                const std::vector<double>& rowYs) {
    int siteWidth = rowMgr->getSiteWidth();
    int siteHeight = rowMgr->getSiteHeight();
    double minX = rowMgr->getRows()[0]->getLoc().getX();
    double maxX = minX + siteWidth * rowMgr->getNumSites();
    auto& occupiedSite = rowMgr->getOccupiedSites();

    for (auto& FF_inst : cellMgr->getInstByType(Cell::Type::FF)) {
        double valid_x = std::min(std::max(FF_inst->getLoc().getX(), minX), maxX);
        double valid_y = std::min(std::max(FF_inst->getLoc().getY(), rowYs.front()), rowYs.back());

        auto lower_x = std::lower_bound(rowXs.begin(), rowXs.end(), valid_x);
        auto lower_y = std::lower_bound(rowYs.begin(), rowYs.end(), valid_y);

        assert(lower_x != rowXs.end() && lower_y != rowYs.end());

        if (*lower_x == FF_inst->getLoc().getX() &&
            *lower_y == FF_inst->getLoc().getY() &&
            occupiedSite.ableToPlace(FF_inst->getCell(), FF_inst->getLoc())) {
            occupiedSite.place(FF_inst->getCell(), FF_inst->getLoc());
        } else {
            Coord nearest_site = findNearestSite(FF_inst.get(), rowXs, rowYs, lower_x, lower_y, siteWidth, siteHeight);
            if (nearest_site.getX() != DBL_MAX) {  // found a valid site
                occupiedSite.place(FF_inst->getCell(), nearest_site);
                FF_inst->setLoc(nearest_site);
            } else {
                std::cerr << "Unable to place FF " << FF_inst->getName() << std::endl;
                exit(1);
            }
        }
    }
}

Coord Legalizer::findNearestSite(Instance* FF_inst,
                                 const std::vector<double>& rowXs,
                                 const std::vector<double>& rowYs,
                                 std::vector<double>::const_iterator lower_x,
                                 std::vector<double>::const_iterator lower_y,
                                 int siteWidth,
                                 int siteHeight) {
    int rangeY = 100;
    int rangeX = rangeY * (siteHeight / siteWidth);

    auto start_x = (std::distance(rowXs.begin(), lower_x) > rangeX)
                       ? lower_x - rangeX
                       : rowXs.begin();
    auto end_x = (std::distance(lower_x, rowXs.end()) > rangeX)
                     ? lower_x + rangeX
                     : rowXs.end();
    auto start_y = (std::distance(rowYs.begin(), lower_y) > rangeY)
                       ? lower_y - rangeY
                       : rowYs.begin();
    auto end_y = (std::distance(lower_y, rowYs.end()) > rangeY)
                     ? lower_y + rangeY
                     : rowYs.end();

    Coord nearest_site(DBL_MAX, DBL_MAX);
    auto& occupiedSite = rowMgr->getOccupiedSites();

    for (auto x = start_x; x != end_x; ++x) {
        for (auto y = start_y; y != end_y; ++y) {
            Coord site(*x, *y);
            if (occupiedSite.ableToPlace(FF_inst->getCell(), site) &&
                nearest_site.Mdist(FF_inst->getLoc()) > site.Mdist(FF_inst->getLoc())) {
                nearest_site = site;
            }
        }
    }
    return nearest_site;
}
