#include "Parser.hpp"

#include "../Manager/DataMgr/DataMgr.hpp"
#include "../global/global.hpp"


// Alpha 10
// Beta 2000
// Gamma 0.002
// Lambda 10000
// DieSize 0 0 1299990 1302000
void Parser::readFactors() {
    double alpha, beta, gamma, lambda;

    util->getIss();
    util->iss >> util->identifier >> alpha;
    util->getIss();
    util->iss >> util->identifier >> beta;
    util->getIss();
    util->iss >> util->identifier >> gamma;
    util->getIss();
    util->iss >> util->identifier >> lambda;
    eval.initialize(alpha, beta, gamma, lambda);

    double lx, ly, rx, ry;
    util->getIss();
    util->iss >> util->identifier >> lx >> ly >> rx >> ry;

    double width = rx - lx;
    double height = ry - ly;
    eval.die->initialize(Coord(lx, ly), width, height);
}


// NumInput 256
// Input INPUT1 1253995 0
// Input INPUT2 1296040 1140825
// Input INPUT3 1296040 1187185
// .
// .
// .
// NumOutput 267
// Output OUTPUT1 1296040 41105
// Output OUTPUT2 1296040 31985
// Output OUTPUT3 1296040 22865
void Parser::readIOpins(InstPinMap &instPinMap) {
    int num;
    util->getIss();
    util->iss >> util->identifier >> num;
    cellMgr->getIOInst()->resizeIOInstInputPins(num);

    for (int i = 0; i < num; i++) {
        std::string input_name;
        double x, y;
        util->getIss();
        util->iss >> util->identifier >> input_name >> x >> y;

        auto cellPin = std::make_unique<Pin>(input_name, i, Coord(x, y), Pin::Type::INPUT);
        auto instPin = std::make_unique<Pin>(input_name, i, Coord(x, y), Pin::Type::INPUT);
        auto raw_instPin = instPin.get();
        cellMgr->addPin2Cell(std::move(cellPin));
        cellMgr->getIOInst()->addInputPin(std::move(instPin), i);
        instPinMap[cellMgr->getIOInst()][input_name] = raw_instPin;
    }

    util->getIss();
    util->iss >> util->identifier >> num;
    cellMgr->getIOInst()->resizeIOInstOutputPins(num);
    for (int i = 0; i < num; i++) {
        std::string output_name;
        double x, y;
        util->getIss();
        util->iss >> util->identifier >> output_name >> x >> y;

        auto cellPin = std::make_unique<Pin>(output_name, i, Coord(x, y), Pin::Type::OUTPUT);
        auto instPin = std::make_unique<Pin>(output_name, i, Coord(x, y), Pin::Type::OUTPUT);
        auto raw_instPin = instPin.get();
        cellMgr->addPin2Cell(std::move(cellPin));
        cellMgr->getIOInst()->addOutputPin(std::move(instPin), i);
        instPinMap[cellMgr->getIOInst()][output_name] = raw_instPin;
    }

    // data->cell_lib.back()->adjustPinID();
}

// FlipFlop 1 FF1 5610 4200 3 (1 bit, name, width, height, pin count)
// Pin Q 1110 3640
// Pin D 1345 400
// Pin CLK 1875 980

// Gate G1 4590 4200 5
// Pin OUT1 90 1260
// Pin OUT2 90 3640
// Pin IN1 3215 1040
// Pin IN2 3895 760
// Pin IN3 90 3080
void Parser::readCellLibrary(CellMap &cellMap) {
    util->getIss();
    util->iss >> util->identifier;
    // for case-insensitive string comparison
    util->toLowerCase();
    while (tolower(util->identifier[0]) == 'f' or tolower(util->identifier[0]) == 'g') {
        int pin_count, bits;
        double w, h;
        bool isFF = false;
        std::string cell_name;

        if (tolower(util->identifier[0]) == 'f') {
            isFF = true;
            util->iss >> bits >> cell_name >> w >> h >> pin_count;
            cellMgr->addFF2Lib(std::make_unique<FlipFlop>(cell_name, w, h, pin_count, bits), cellMap);
        } else {
            util->iss >> cell_name >> w >> h >> pin_count;
            cellMgr->addGate2Lib(std::make_unique<Gate>(cell_name, w, h, pin_count), cellMap);
        }
        while (pin_count--) {
            std::string pin_name;
            double x, y;
            util->getIss();
            util->iss >> util->identifier >> pin_name >> x >> y;
            int ID;
            switch (toupper(pin_name[0])) {
                case 'C': {  // CLK (FF)
                    ID = 0;
                    cellMgr->addPin2Cell(
                        std::make_unique<Pin>(pin_name, ID, Coord(x, y), Pin::Type::CLK));
                    break;
                }
                case 'D': {  // input pin (FF)
                    ID = (pin_name.size() == 1)
                             ? 0
                             : std::stoi(pin_name.substr(1));
                    cellMgr->addPin2Cell(
                        std::make_unique<Pin>(pin_name, ID, Coord(x, y), Pin::Type::D));
                    break;
                }
                case 'Q': {  // output pin (FF)
                    ID = (pin_name.size() == 1)
                             ? 0
                             : std::stoi(pin_name.substr(1));
                    cellMgr->addPin2Cell(
                        std::make_unique<Pin>(pin_name, ID, Coord(x, y), Pin::Type::Q));
                    break;
                }
                case 'I': {  // input pin (Gate)
                    ID = (pin_name.size() == 2)
                             ? 0
                             : std::stoi(pin_name.substr(2)) - 1;
                    cellMgr->addPin2Cell(
                        std::make_unique<Pin>(pin_name, ID, Coord(x, y), Pin::Type::IN));
                    break;
                }
                case 'O': {  // output pin (Gate)
                    ID = (pin_name.size() == 3)
                             ? 0
                             : std::stoi(pin_name.substr(3)) - 1;
                    cellMgr->addPin2Cell(std::make_unique<Pin>(pin_name, ID, Coord(x, y), Pin::Type::OUT));
                    break;
                }
                default: {
                    std::cerr << "[Error] Error in input file: Unrecognizable pin name \""
                              << pin_name << "\".\n";
                    std::cerr << "This error occurs in reading " << util->identifier << " " << pin_name << " " << x << " " << y << ".\n";
                    exit(EXIT_FAILURE);
                    break;
                }
            }
        }

        // if the output pins are not given after all input pins are given, need adjustment to ensure correct pinID
        cellMgr->adjustPinID(isFF);
        util->getIss();
        util->iss >> util->identifier;
        util->toLowerCase();
    }
}

// NumInstances 108685
// Inst C1 G322 304470 661500 (name, cell type, x, y)
// Inst C2 G220 1117920 690900
// Inst C3 G115 442680 693000
void Parser::readInstances(InstMap &instMap, CellMap &cellMap, InstPinMap &instPinMap) {
    int numInstances;
    std::string identifier;

    util->iss >> numInstances;
    // cellMgr->gate_instances.reserve(numInstances);
    cellMgr->gate_instances.reserve(133000);
    cellMgr->FF_instances.reserve(22000);
    while (numInstances--) {
        std::string cell_name, inst_name;
        double x, y;
        util->getIss();
        util->iss >> identifier >> inst_name >> cell_name >> x >> y;
        Cell *cell = cellMap[cell_name];

        size_t instID = (cell->getType() == Cell::Type::FF)
                            ? cellMgr->genInstID()
                            : 0;

        cellMgr->addInst(
            instMap, instPinMap,
            std::make_unique<Instance>(inst_name, Coord(x, y), cell, instID));
    }
}

// NumNets 104235
// Net net36760 5
// Pin C18117/OUT1 (cell name, pin name)
// Pin C46854/IN4
// Pin C46930/IN4
// Pin C46993/IN4
// Pin C47056/IN4
void Parser::readNets(InstMap &instMap, InstPinMap &instPinMap) {
    int numNets;

    util->getIss();
    util->iss >> util->identifier >> numNets;
    while (numNets--) {
        std::string net_name;
        int netPin_count;
        util->getIss();

        util->iss >> util->identifier >> net_name >> netPin_count;
        std::unique_ptr<Net> net = std::make_unique<Net>(net_name);

        while (netPin_count--) {
            std::string inst_pin, inst_name, pin_name;
            util->getIss();

            util->iss >> util->identifier >> inst_pin;

            std::size_t slashPos = inst_pin.find('/');
            Instance *inst;
            if (slashPos != std::string::npos) {
                inst_name = inst_pin.substr(0, slashPos);
                pin_name = inst_pin.substr(slashPos + 1);
                inst = instMap[inst_name];
            } else {
                pin_name = inst_pin;
                inst = cellMgr->getIOInst();
            }
            auto pin = instPinMap[inst][pin_name];
            net->addPin2Net(pin);
            pin->setNet(net.get());
        }
        cellMgr->addNet(std::move(net));
    }
}


// BinWidth 29400
// BinHeight 29400
// BinMaxUtil 92.93
// PlacementRows 15300 16800 510 2100 2489
// PlacementRows 15300 18900 510 2100 2489
// 今年沒有bin density
void Parser::readBin_Row_displacementDelay() {
    double binWidth, binHeight, binMaxUtil;
    util->getIss();
    util->iss >> util->identifier >> binWidth;
    util->getIss();
    util->iss >> util->identifier >> binHeight;
    util->getIss();
    util->iss >> util->identifier >> binMaxUtil;
    eval.setBin(binWidth, binHeight, binMaxUtil);

    util->getIss();
    util->iss >> util->identifier;
    util->toLowerCase();

    while (util->identifier == "placementrows") {
        double x, y;
        int siteWidth, siteHeight;
        int NumSites;
        util->iss >> x >> y >> siteWidth >> siteHeight >> NumSites;
        data->rowMgr->addRow(std::make_unique<Row>(Coord(x, y), siteHeight, siteWidth, NumSites));

        util->getIss();
        util->iss >> util->identifier;
        util->toLowerCase();
    }

    double dsplcDelay;
    util->iss >> dsplcDelay;
    eval.dsplcDelay = dsplcDelay;
}

// DisplacementDelay 0.001
// QpinDelay FF1 18.052032
// .
// .
// TimingSlack C41831 D 11.995196
// .
// .
// GatePower FF1 0.019300
void Parser::readQdelay_Slack_Power(CellMap &cellMap, InstMap &instMap) {
    util->getIss();
    util->iss >> util->identifier;
    util->toLowerCase();

    while (util->identifier == "qpindelay") {
        std::string cell_name;
        double QpinDelay;

        util->iss >> cell_name >> QpinDelay;
        cellMgr->setQpinDelay(cell_name, QpinDelay, cellMap[cell_name]);

        util->getIss();
        util->iss >> util->identifier;
        util->toLowerCase();
    }

    while (util->identifier == "timingslack") {
        std::string inst_name, pin_name;
        double slack;

        util->iss >> inst_name >> pin_name >> slack;
        cellMgr->setTimingSlack(instMap[inst_name], pin_name, slack);

        util->getIss();
        util->iss >> util->identifier;
        util->toLowerCase();
    }

    while (util->identifier == "gatepower" and util->fin) {
        std::string cell_name;
        double power;

        util->iss >> cell_name >> power;
        cellMgr->setPower(cellMap[cell_name], power);

        util->getIss();
        util->iss >> util->identifier;
        util->toLowerCase();
    }
}

void Parser::readInput(const std::string &inputPath) {
    ParserUtil util(inputPath);
    InstMap instMap;
    CellMap cellMap;
    InstPinMap instPinMap;

    readFactors();
    readIOpins(instPinMap);
    readCellLibrary(cellMap);
    readInstances(instMap, cellMap, instPinMap);
    readNets(instMap, instPinMap);
    readBin_Row_displacementDelay();
    readQdelay_Slack_Power(cellMap, instMap);
}

std::unique_ptr<DataMgr> Parser::parse(ArgumentParser& argParser) {
#ifdef DEBUG_MODE
    timer.startTimer("parsing");
#endif
    util = new ParserUtil(argParser.inputPath);
    inputPath = argParser.inputPath;
    auto dataMgr = std::make_unique<DataMgr>(argParser.outputPath);
    data = dataMgr.get();
    cellMgr = data->cellMgr.get();
    readInput(inputPath);

#ifdef DEBUG_MODE
    timer.stopTimer("parsing");
#endif
    return dataMgr;
}
