#pragma once
#include <memory>

#include "../Coord/Coord.hpp"

class Row {
   public:
    using ptr = std::unique_ptr<Row>;
    Row() : loc(Coord(0, 0)),
            siteHeight(0),
            siteWidth(0),
            numSites(0) {}
    Row(Coord loc, int siteHeight, int siteWidth, int numSites)
        : loc(loc),
          siteHeight(siteHeight),
          siteWidth(siteWidth),
          numSites(numSites) {}

    const Coord &getLoc() const { return loc; }
    int getSiteHeight() const { return siteHeight; }
    int getSiteWidth() const { return siteWidth; }
    int getNumSites() const { return numSites; }

   private:
    Coord loc;
    int siteHeight, siteWidth, numSites;
};
