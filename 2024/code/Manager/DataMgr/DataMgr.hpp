#pragma once

#include <fstream>
#include <limits>
#include <memory>
#include <random>
#include <vector>

#include "../../Structure/Die/Die.hpp"
#include "../../Structure/Net/Net.hpp"
#include "../../Structure/Pin/Pin.hpp"
#include "../CellMgr/CellMgr.hpp"
#include "../RowMgr/RowMgr.hpp"

class Cell;

class DataMgr {
   public:
    using ptr = std::unique_ptr<DataMgr>;
    using str = std::string;

    DataMgr(std::string& outputPath);
    void output() const;

    std::unique_ptr<CellMgr> cellMgr;
    std::unique_ptr<RowMgr> rowMgr;
    std::string outputPath;
};
