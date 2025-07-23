#include "RowMgr.hpp"

void RowMgr::addRow(Row::ptr row) {
    Lbnd = std::min(Lbnd, row->getLoc().getX());
    Rbnd = std::max(Rbnd, row->getLoc().getX() + row->getNumSites() * row->getSiteWidth());
    Dbnd = std::min(Dbnd, row->getLoc().getY());
    Ubnd = std::max(Ubnd, row->getLoc().getY() + row->getSiteHeight());
    siteWidth = row->getSiteWidth();
    siteHeight = row->getSiteHeight();
    numSites = row->getNumSites();
    rows.emplace_back(move(row));
}

void RowMgr::initOccupiedSites() {
    occupiedSites = BitMatrix(rows.size(), rows[0]->getNumSites(),
                              rows[0]->getSiteWidth(), rows[0]->getSiteHeight(),
                              rows[0]->getLoc().getX(), rows[0]->getLoc().getY());
}