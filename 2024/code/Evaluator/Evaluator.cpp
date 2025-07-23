#include "Evaluator.hpp"

#include "../Manager/CellMgr/CellMgr.hpp"
#include "../Structure/Instance/Instance.hpp"

void Evaluator::initialize(double a, double b, double g, double lmb) {
    this->alpha = a;
    this->beta = b;
    this->gamma = g;
    this->lambda = lmb;
}

void Evaluator::setBin(double bw, double bh, double bmu) {
    this->binWidth = bw;
    this->binHeight = bh;
    this->binMaxUtil = bmu;
}

double Evaluator::getScore() const {
    return getTnsScore() +
           getPowerScore() +
           getAreaScore() +
           getDensityScore();
}

std::vector<std::tuple<int, int, double>>
Evaluator::computeBinContrib(Instance* inst, const Coord& loc, int numBinsX, int numBinsY) const {
    std::vector<std::tuple<int, int, double>> contrib;
    int width = inst->getCell()->getWidth();
    int height = inst->getCell()->getHeight();
    double instArea = width * height;

    int instX = loc.getX() - die->getLoc().getX();
    int instY = loc.getY() - die->getLoc().getY();
    int instX_plusW = instX + width;
    int instY_plusH = instY + height;

    int binL = instX / binWidth;
    int binD = instY / binHeight;
    int binR = instX_plusW / binWidth;
    int binU = instY_plusH / binHeight;

    if (binL >= numBinsX or binD >= numBinsY) {
        std::cout << "Bin indices out of bounds: " << binL << ", " << binR
                  << "; derived from instance loc (" << loc.getX() << ", " << loc.getY() << ")\n";
        return contrib;
    }
    if (binL == binR && binD == binU) {
        contrib.push_back({binL, binD, instArea});
    } else {
        int LD = (((binL + 1) * binWidth) - instX) *
                 (((binD + 1) * binHeight) - instY);
        int LU = (((binL + 1) * binWidth) - instX) *
                 (instY_plusH - ((binD + 1) * binHeight));
        int RD = (instX_plusW - ((binL + 1) * binWidth)) *
                 (((binD + 1) * binHeight) - instY);
        int RU = (instX_plusW - ((binL + 1) * binWidth)) *
                 (instY_plusH - ((binD + 1) * binHeight));

        if (binL < binR && binD < binU) {  // Spans four bins.
            contrib.push_back({binL, binD, LD});
            contrib.push_back({binL, binU, LU});
            contrib.push_back({binR, binD, RD});
            contrib.push_back({binR, binU, RU});
        } else if (binL < binR) {  // Spans two bins horizontally.
            LD = (((binR)*binWidth) - instX) * height;
            RD = (instX_plusW - ((binR)*binWidth)) * height;
            contrib.push_back({binL, binD, LD});
            contrib.push_back({binR, binD, RD});
        } else if (binD < binU) {  // Spans two bins vertically.
            LD = width * (((binU)*binHeight) - instY);
            LU = width * (instY_plusH - ((binU)*binHeight));
            contrib.push_back({binL, binD, LD});
            contrib.push_back({binL, binU, LU});
        } else {
            std::cout << "Unexpected case in shiftInst: instance spans bins unusually.\n";
        }
    }
    return contrib;
}

inline double Evaluator::getAllowedArea(int i, int j, int numBinsX, int numBinsY) const {
    if (i == numBinsX - 1 or j == numBinsY - 1) {
        return std::min(binWidth, die->width - binWidth * i) *
               std::min(binHeight, die->height - binHeight * j);
    } else {
        return binWidth * binHeight * binMaxUtil / 100.0;
    }
}

void Evaluator::shiftInst(Instance* inst, const Coord& newLoc) {
    int numBinsX = std::ceil(die->width / binWidth);
    int numBinsY = std::ceil(die->height / binHeight);

    auto oldContrib = computeBinContrib(inst, inst->getLoc(), numBinsX, numBinsY);
    auto newContrib = computeBinContrib(inst, newLoc, numBinsX, numBinsY);

    std::map<std::pair<int, int>, double> binDeltas;
    for (const auto& [binX, binY, contribArea] : oldContrib) {
        binDeltas[{binX, binY}] -= contribArea;
    }
    for (const auto& [binX, binY, contribArea] : newContrib) {
        binDeltas[{binX, binY}] += contribArea;
    }

    for (auto& [binXY, delta] : binDeltas) {
        auto& [binX, binY] = binXY;
        double oldBinArea = binAreas[binX][binY];
        bool wasViolated = (oldBinArea > getAllowedArea(binX, binY, numBinsX, numBinsY));

        binAreas[binX][binY] += delta;
        bool nowViolated = (binAreas[binX][binY] > getAllowedArea(binX, binY, numBinsX, numBinsY));

        if (wasViolated && !nowViolated)
            violatedBins--;
        else if (!wasViolated && nowViolated)
            violatedBins++;
    }

    inst->setLoc(newLoc);
    inst->updtDlySlk();
}

double Evaluator::initDensity(CellMgr* cellMgr) {
    int numBinsX = ceil(die->width / binWidth);
    int numBinsY = ceil(die->height / binHeight);

    binAreas.resize(numBinsX, std::vector<double>(numBinsY, 0.0));

    for (auto& inst : cellMgr->getInstByType(Cell::Type::FF)) {
        add2Bin(inst.get());
    }
    for (auto& inst : cellMgr->getInstByType(Cell::Type::GATE)) {
        add2Bin(inst.get());
    }

    double maxAllowedArea = binWidth * binHeight * binMaxUtil / 100.0;  // Convert percentage to fraction
    for (int i = 0; i < numBinsX; ++i) {
        for (int j = 0; j < numBinsY; ++j) {
            if (i == numBinsX - 1 or j == numBinsY - 1) {
                double partialMaxAllowedArea = std::min(binWidth, die->width - binWidth * i) *
                                               std::min(binHeight, die->height - binHeight * j);
                if (binAreas[i][j] > partialMaxAllowedArea) {
                    violatedBins++;
                }
            } else if (binAreas[i][j] > maxAllowedArea) {
                violatedBins++;
            }
        }
    }

    return getDensityScore();
}

void Evaluator::add2Bin(Instance* inst) {
    int numBinsX = ceil(die->width / binWidth);
    int numBinsY = ceil(die->height / binHeight);

    int width = inst->getCell()->getWidth();
    int height = inst->getCell()->getHeight();
    const Coord& loc = inst->getLoc();
    double area = width * height;

    int instX = ((loc.getX() - this->die->getLoc().getX()));
    int instY = (loc.getY() - this->die->getLoc().getY());
    int instX_addW = instX + width;
    int instY_addH = instY + height;

    int binL = int(instX / binWidth);
    int binD = int(instY / binHeight);
    int binR = int(instX_addW / binWidth);
    int binU = int(instY_addH / binHeight);

    // Ensure the bin indices are within the array bounds
    if (binL >= numBinsX or binD >= numBinsY) {
        std::cout << "Bin indices out of bounds: " << binL << ", " << binR
                  << "; derived from inst loc " << loc.getX() << ", " << loc.getY()
                  << "\n";
        return;
    }

    if (binL == binR and binD == binU) {
        binAreas[binL][binD] += area;
    } else {
        int LD = (((binL + 1) * binWidth) - instX) *
                 (((binD + 1) * binHeight) - instY);
        int LU = (((binL + 1) * binWidth) - instX) *
                 (instY_addH - ((binD + 1) * binHeight));
        int RD = (instX_addW - ((binL + 1) * binWidth)) *
                 (((binD + 1) * binHeight) - instY);
        int RU = (instX_addW - ((binL + 1) * binWidth)) *
                 (instY_addH - ((binD + 1) * binHeight));

        if (binL < binR and binD < binU) {
            binAreas[binL][binD] += LD;
            binAreas[binL][binU] += LU;
            binAreas[binR][binD] += RD;
            binAreas[binR][binU] += RU;
        } else if (binL < binR) {
            LD = (((binR) * binWidth) - instX) * height;
            RD = (instX_addW - ((binR) * binWidth)) * height;
            binAreas[binL][binD] += LD;
            binAreas[binR][binD] += RD;
        } else if (binD < binU) {
            LD = width * (((binU) * binHeight) - instY);
            LU = width * (instY_addH - ((binU) * binHeight));
            binAreas[binL][binD] += LD;
            binAreas[binL][binU] += LU;
        } else {
            std::cout << "There should be no case where binX > binRight and binY > binUp\n";
        }
    }
}

void Evaluator::printEvaluationInfo() {
    double tnsScore = 0.0, powerScore = 0.0, areaScore = 0.0, density = 0.0;
    tnsScore = getTnsScore();
    powerScore = getPowerScore();
    areaScore = getAreaScore();
    density = getDensityScore();
    double weightedScore = tnsScore + powerScore + areaScore + density;

    std::cout << "\n=========== Evaluation result =========== " << "\n";
    std::cout << std::setprecision(10)
              << "weighted TNS:   " << alpha << " * " << -TNS << " = " << tnsScore
              << "(" << std::fixed << std::setprecision(2) << (tnsScore / weightedScore) * 100 << "%)\n"

              << std::setprecision(10) << std::defaultfloat
              << "weighted Power: " << beta << " * " << power << " = " << powerScore
              << "(" << std::fixed << std::setprecision(2) << (powerScore / weightedScore) * 100 << "%)\n"

              << std::setprecision(10) << std::defaultfloat
              << "weighted Area: " << gamma << " * " << area << " = " << areaScore
              << "(" << std::fixed << std::setprecision(2) << (areaScore / weightedScore) * 100 << "%)\n"

              << std::setprecision(10) << std::defaultfloat
              << "weighted Density: " << lambda << " * " << violatedBins << " = " << density
              << "(" << std::fixed << std::setprecision(2) << (density / weightedScore) * 100 << "%)\n"

              << "\nInitial score: " << weightedScore << "\n";

    std::cout.unsetf(std::ios::fixed);  // Reset formatting after output
}
