#include "DataMgr.hpp"

#include "../../global/global.hpp"

DataMgr::DataMgr(std::string &outputPath)
    : cellMgr(std::make_unique<CellMgr>()),
      rowMgr(std::make_unique<RowMgr>()),
      outputPath(outputPath) {
}

void DataMgr::output() const {
#ifdef DEBUG_MODE
    timer.startTimer("output");
#endif
    std::ofstream outputFile;
    outputFile.open(outputPath);

    outputFile << "CellInst " << cellMgr->getInstByType(Cell::Type::FF).size() << '\n';
    for (auto &FF_inst : cellMgr->getInstByType(Cell::Type::FF)) {
        outputFile << std::fixed
                   << std::setprecision(0)
                   << "Inst "
                   << "F" << FF_inst->getID() << " "
                   << FF_inst->getCell()->getName() << " "
                   << FF_inst->getLoc() << '\n';
    }
    for (auto &FF_inst : cellMgr->getInstByType(Cell::Type::FF)) {
        std::string inst_name = FF_inst->getName();
        bool oneBitFF = FF_inst->getInputPins().size() == 1;
        for (auto &pin : FF_inst->getInputPins()) {
            // if(!pin) continue;
            auto &[oldInst, oldPin] = cellMgr->getOldNode(pin->getTag());
            outputFile << oldInst << "/"
                       << oldPin << " map "
                       << "F" << FF_inst->getID() << "/D"
                       << ((oneBitFF) ? "" : std::to_string(pin->getID()))
                       << '\n';
        }
        for (auto &pin : FF_inst->getOutputPins()) {
            // if(!pin) continue;
            auto &[oldInst, oldPin] = cellMgr->getOldNode(pin->getTag());
            outputFile << oldInst << "/"
                       << oldPin << " map "
                       << "F" << FF_inst->getID() << "/Q"
                       << ((oneBitFF) ? "" : std::to_string(pin->getID()))
                       << '\n';
        }
        for (auto &pin : FF_inst->getCLKPins()) {
            auto &[oldInst, oldPin] = cellMgr->getOldNode(pin->getTag());
            outputFile << oldInst << "/"
                       << oldPin << " map "
                       << "F" << FF_inst->getID() << "/CLK"
                       << '\n';
        }
    }

    outputFile.flush();
    outputFile.close();
#ifdef DEBUG_MODE
    timer.stopTimer("output");
#endif
}