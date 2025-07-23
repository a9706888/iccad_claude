#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

#include "ArgumentParser.hpp"
#include "ParserUtil.hpp"

class Cell;
class Pin;
class DataMgr;
class CellMgr;
class Instance;

class Parser {
    std::string inputPath;
    ParserUtil *util;
    DataMgr *data;
    CellMgr *cellMgr;
    using InstMap = std::unordered_map<std::string, Instance *>;
    using CellMap = std::unordered_map<std::string, Cell *>;
    using InstPinMap = std::unordered_map<Instance *, std::unordered_map<std::string, Pin *>>;

    void readInput(const std::string &inputPath);
    void readFactors();
    void readIOpins(InstPinMap &instPinMap);
    void readCellLibrary(CellMap &cellMap);
    void readInstances(InstMap &instMap, CellMap &cellMap, InstPinMap &instPinMap);
    void readNets(InstMap &instMap, InstPinMap &instPinMap);
    void readBin_Row_displacementDelay();
    void readQdelay_Slack_Power(CellMap &cellMap, InstMap &instMap);

   public:
    Parser() = default;
    std::unique_ptr<DataMgr> parse(ArgumentParser &argParser);
    ~Parser() { delete util; };
};
