#include "Solver.hpp"

Solver::Solver(DataMgr* data)
    : cellMgr(data->cellMgr.get()),
      rowMgr(data->rowMgr.get()),
      siteWidth(rowMgr->getSiteWidth()),
      siteHeight(rowMgr->getSiteHeight()) {
}

void Solver::preprocessing() {
    cellMgr->preprocess();

    Legalizer(cellMgr, rowMgr).legalize();

    for (auto& G_inst : cellMgr->getInstByType(Cell::Type::GATE)) {
        allInstBoxes.insert(Box(G_inst.get()));
    }
    bestFFOfEachSize = buildBestFFEachSize();
    // cellMgr->printLowerBound(bestFFOfEachSize);

    fillDieWithBestFFs();
}

void Solver::solve() {
#ifdef DEBUG_MODE
    timer.startTimer("solve");
#endif
    auto& FF_instances = cellMgr->getInstByType(Cell::Type::FF);

    preprocessing();
    debanking();
    banking();

    eval.initDensity(cellMgr);

#ifdef DEBUG_MODE
    std::cout << "After debanking & banking:\n";
    cellMgr->printCost();
#endif

    double prevScore = eval.getScore();

    for (int iter = 1;; iter++) {
#ifdef DEBUG_MODE
        std::cout << "iteration " << iter << "\n";
#endif
        for (auto& FF_inst : FF_instances) {
            FFSwap(FF_inst.get());
        }
        for (auto& FF_inst : FF_instances) {
            FFRelocate(FF_inst.get());
        }
        for (auto& FF_inst : FF_instances) {
            pinSwap(FF_inst.get());
        }
        for (auto& FF_inst : FF_instances) {
            FF_inst->pinPermute();
        }

        // checkOverlap();
#ifdef DEBUG_MODE
        cellMgr->printCost();
#endif
        double improv = (prevScore - eval.getScore()) / prevScore;
        if (improv < CNVRGTHRESHOLD) {
            break;
        }
        prevScore = eval.getScore();
    }
    // cellMgr->genSlack();

#ifdef DEBUG_MODE
    timer.stopTimer("solve");
    printParas();
    cellMgr->printFFcnt();
#endif
}

inline int round(int val, int unit) { return (val / unit) * unit; }
inline int Ceil(double val) { return (int)ceil(val); }

void Solver::FFRelocate(Instance* FF_inst) {
    auto cell = FF_inst->getCell();
    auto originLoc = FF_inst->getLoc();

    auto legalCenLoc = Coord(round(cell->getWidth() / 2, siteWidth),
                             round(cell->getHeight() / 2, siteHeight));
    auto lwrLftCorner = Coord((Ceil(MOVE_SIDE * SQRT2) * siteWidth),
                              round(Ceil(MOVE_SIDE * SQRT2) * siteWidth, siteHeight));
    auto baseLoc = originLoc + legalCenLoc - lwrLftCorner;

    Coord::CoordSet jumpLocs;

    int randomTimes = RDMSHIFTTIMES;
    int modW = (2 * Ceil(MOVE_SIDE * SQRT2)) - (cell->getWidth() / siteWidth) + 1;
    int modH = (2 * Ceil(MOVE_SIDE * SQRT2) * siteWidth / siteHeight) - (cell->getHeight() / siteHeight) + 1;

    while (randomTimes--) {
        int x = distr1000(generator) % modW;
        int y = distr1000(generator) % modH;
        Coord shiftLoc = baseLoc + Coord(x * siteWidth, y * siteHeight);

        if (originLoc == shiftLoc) continue;

        Box shiftBox(shiftLoc, FF_inst);

        if (outOfDie(shiftBox)) continue;

        std::vector<Box> touchedBoxes;
        allInstBoxes.query(bgi::intersects(shiftBox), std::back_inserter(touchedBoxes));
        bool shiftLocValid = !std::any_of(touchedBoxes.begin(), touchedBoxes.end(),
                                          [this, FF_inst, &shiftBox](const Box& box) {
                                              return (box.inst != FF_inst) and
                                                     (getBoxCen(box).Mdist(getBoxCen(shiftBox)) > MOVE_SIDE * siteWidth or
                                                      bg::within(box, shiftBox) or
                                                      bg::within(shiftBox, box) or
                                                      bg::overlaps(box, shiftBox));
                                          });
        if (shiftLocValid) jumpLocs.insert(shiftLoc);
    }
    Coord newLoc = cellMgr->relocate(FF_inst, jumpLocs);
    if (newLoc != originLoc) {
        allInstBoxes.insert(Box(FF_inst));
        allInstBoxes.remove(Box(originLoc, FF_inst));
    }
}

void Solver::FFSwap(Instance* FF_inst) {
    Coord FF_center = FF_inst->getCenLoc();
    double radius = MOVE_SIDE * siteWidth;
    Box searchArea(
        FF_center - Coord(radius * SQRT2, radius * SQRT2),
        FF_center + Coord(radius * SQRT2, radius * SQRT2));

    std::vector<Box> touchedBoxes;
    allInstBoxes.query(bgi::intersects(searchArea), std::back_inserter(touchedBoxes));

    std::vector<Box> overlapBoxes;
    for (auto& box : touchedBoxes) {
        if (bg::within(box, searchArea) or bg::overlaps(box, searchArea)) {
            if (box.inst->getCenLoc().Mdist(FF_center) <= MOVE_SIDE * siteWidth)
                overlapBoxes.push_back(box);
        }
    }

    std::vector<Instance*> swapCddts;
    for (auto& box : overlapBoxes) {
        auto nearbyFF = box.inst;
        if (nearbyFF->isType(Cell::Type::FF) and
            nearbyFF->sameShape(FF_inst) and
            nearbyFF != FF_inst) {
            swapCddts.push_back(nearbyFF);
        }
    }
    if (swapCddts.empty()) return;

    Instance* swapInst = nullptr;
    double bestTNS = DBL_MAX;

    for (auto& swapCddt : swapCddts) {
        double swapTNS = cellMgr->getSwapTNS(FF_inst, swapCddt);
        if (swapTNS < bestTNS) {
            bestTNS = swapTNS;
            swapInst = swapCddt;
        }
    }

    if (!swapInst) return;

    Box FF_instBox(FF_inst->getLoc(), FF_inst);
    Box swapInstBox(swapInst->getLoc(), swapInst);

    FF_inst->swapLoc(swapInst);

    allInstBoxes.remove(FF_instBox);
    allInstBoxes.remove(swapInstBox);

    swapInstBox.inst = FF_inst;
    FF_instBox.inst = swapInst;

    allInstBoxes.insert(swapInstBox);
    allInstBoxes.insert(FF_instBox);
}

void Solver::checkOverlap() {
    for (auto& FF_inst : cellMgr->getInstByType(Cell::Type::FF)) {
        std::vector<Box> touchedBoxes;
        Box FFBox(FF_inst->getLoc(), FF_inst.get());
        allInstBoxes.query(bgi::intersects(FFBox), std::back_inserter(touchedBoxes));
        if (touchedBoxes.size() <= 1) continue;

        std::vector<Box> intersectBoxes;
        for (auto& box : touchedBoxes) {
            if (bg::within(box, FFBox) or bg::overlaps(box, FFBox)) {
                intersectBoxes.push_back(box);
            }
        }
        bool Overlap = false;
        for (auto& box : intersectBoxes) {
            Overlap |= !bg::within(box, FFBox);
            if (Overlap) break;
        }

        if (Overlap) {
            std::cout << "Overlap detected! FF: " << FF_inst->getName()
                      << " at ("
                      << FF_inst->getLoc().getX() << ", "
                      << FF_inst->getLoc().getY() << ")\n";

            std::cout << "Overlapping Instances:\n";
            for (auto& box : intersectBoxes) {
                if (!bg::within(box, FFBox)) {
                    std::cout << "Instance: " << box.inst->getName()
                              << " at ("
                              << box.inst->getLoc().getX() << ", "
                              << box.inst->getLoc().getY() << ")\n";
                }
            }
            std::cout << "\n";
        }
    }
}

std::vector<FlipFlop*> Solver::buildBestFFEachSize() {
    std::vector<FlipFlop*> sortedFFLibCells;
    for (auto& FF_cell : cellMgr->getFFLibCells()) {
        sortedFFLibCells.push_back(FF_cell.get());
    }
    sortedFFLibCells.erase(unique(sortedFFLibCells.begin(), sortedFFLibCells.end()), sortedFFLibCells.end());

    std::sort(sortedFFLibCells.begin(), sortedFFLibCells.end(), [](FlipFlop* a, FlipFlop* b) {
        return a->calcScore() < b->calcScore();
    });

    std::vector<FlipFlop*> bestFFOfEachSize;
    std::unordered_set<int> seenBits;
    for (auto& FF : sortedFFLibCells) {
        if (!seenBits.count(FF->getBits())) {
            bestFFOfEachSize.push_back(FF);
            seenBits.insert(FF->getBits());
        }
    }
    for (size_t i = 1; i < bestFFOfEachSize.size(); ++i) {
        if (bestFFOfEachSize[i - 1]->getBits() < bestFFOfEachSize[i]->getBits()) {
            bestFFOfEachSize.resize(i);
            break;
        }
    }

    for (auto& FF : bestFFOfEachSize) {
        shape2Size[{FF->getWidth(), FF->getHeight()}] = FF->getBits();
    }
    return bestFFOfEachSize;
}

void Solver::fillDieWithBestFFs() {
    rowMgr->initOccupiedSites();
    for (auto& G_inst : cellMgr->getInstByType(Cell::Type::GATE)) {
        rowMgr->getOccupiedSites().place(G_inst->getCell(), G_inst->getLoc());
    }

    auto& occupiedSite = rowMgr->getOccupiedSites();
    for (auto& FF : bestFFOfEachSize) {
        int x_end = rowMgr->getRbnd() - FF->getWidth();
        int y_end = rowMgr->getUbnd() - FF->getHeight();

        for (int y = rowMgr->getDbnd(); y <= y_end; y += siteHeight) {
            for (int x = rowMgr->getLbnd(); x <= x_end;) {
                Coord siteLoc(x, y);

                if (occupiedSite.ableToPlace(FF, siteLoc)) {
                    occupiedSite.place(FF, siteLoc);
                    FFShellBoxes.insert(Box(
                        Coord(x, y),
                        Coord(x + FF->getWidth(),
                              y + FF->getHeight()),
                        nullptr));
                    x += FF->getWidth();
                } else {
                    x += siteWidth;
                }
            }
        }
    }
}

void Solver::debanking() {
    std::vector<Instance::ptr> newFFInsts;
    for (auto& FF_inst : cellMgr->getInstByType(Cell::Type::FF)) {
        if (FF_inst->isDeleted()) continue;

        int n = FF_inst->getInputPins().size();
        if (n == 1) continue;
        
        auto oneBitFF = bestFFOfEachSize.back();
        assert(oneBitFF->getBits() == 1);

        std::vector<Coord> alignOutpinLocs;
        for (auto& outPin : FF_inst->getOutputPins()) {
            alignOutpinLocs.emplace_back(outPin->getLoc() - oneBitFF->getOutputPin(0)->getLoc());
        }


        auto clkPin = FF_inst->getCLKPin(0);
        for (int i = 0; i < n; ++i) {
            auto split_FF = cellMgr->splitFF(FF_inst.get(), oneBitFF, alignOutpinLocs[i], i, clkPin);
            newFFInsts.emplace_back(std::move(split_FF));
        }
        FF_inst->setDeleted(true);
    }
    for (auto& newFF : newFFInsts) {
        cellMgr->addCreatedInst(std::move(newFF));
    }
    cellMgr->rmvAndDltInst();
}

void Solver::banking() {
    BoxRTree FFinstBoxesOnly;
    for (const auto& FF_inst : cellMgr->getInstByType(Cell::Type::FF)) {
        FFinstBoxesOnly.insert(Box(FF_inst.get()));
    }

    std::vector<Instance::ptr> newFFInsts;
    int lefted_FF_cnt = cellMgr->getInstByType(Cell::Type::FF).size();

    for (const auto& FF_inst : cellMgr->getInstByType(Cell::Type::FF)) {
        if (FF_inst->isDeleted()) continue;

        std::vector<FlipFlop*>::iterator bestFitFF_it;
        Box bestShell;
        std::vector<Instance*> merge_ffs;

        for (int shellSearchSide = SHELLSEARCH_SIDE;; ++shellSearchSide) {
            bestFitFF_it = bestFFOfEachSize.begin();

            do {
                bestShell = findBestShell(FF_inst.get(), *bestFitFF_it, shellSearchSide);
                merge_ffs = {FF_inst.get()};

                if ((*bestFitFF_it)->getBits() > 1 and bestShell.min_corner.getX() != INT_MIN) {
                    findRestFFs(bestShell, merge_ffs, FFinstBoxesOnly, shellSearchSide);
                }

            } while (((*bestFitFF_it)->getBits() > lefted_FF_cnt or
                      bestShell.min_corner.getX() == INT_MIN or
                      merge_ffs.size() != (size_t)(*bestFitFF_it)->getBits()) and
                     //  !mtBit2FF.count(merge_ffs.size()) and
                     (++bestFitFF_it) != bestFFOfEachSize.end());

            if (bestFitFF_it != bestFFOfEachSize.end()) break;
        }

        lefted_FF_cnt -= merge_ffs.size();
        auto MBFF_inst = cellMgr->mergeFFs((*bestFitFF_it),
                                           merge_ffs,
                                           bestShell.min_corner);
        newFFInsts.emplace_back(std::move(MBFF_inst));

        FFShellBoxes.remove(bestShell);
        for (auto& FF_inst : merge_ffs) {
            FFinstBoxesOnly.remove(Box(FF_inst));
        }
    }

    for (auto& FF_inst : newFFInsts) {
        allInstBoxes.insert(Box(FF_inst.get()));
        cellMgr->addCreatedInst(std::move(FF_inst));
    }

    cellMgr->rmvAndDltInst();
}

void Solver::findRestFFs(const Box& bestShell,
                         std::vector<Instance*>& merge_ffs,
                         const BoxRTree& FFinstBoxesOnly,
                         int shellSearchSide) {
    const double searchRadius = shellSearchSide * siteWidth;

    Coord boxCen = getBoxCen(bestShell);
    Box searchArea(
        boxCen - Coord(searchRadius * SQRT2, searchRadius * SQRT2),
        boxCen + Coord(searchRadius * SQRT2, searchRadius * SQRT2));

    std::vector<Box> touchedBoxes;
    FFinstBoxesOnly.query(bgi::intersects(searchArea), std::back_inserter(touchedBoxes));
    std::vector<Instance*> rest_candidates;

    auto clkNet = merge_ffs[0]->getCLKPin(0)->getNet();
    for (auto& box : touchedBoxes) {
        if (box.inst != merge_ffs[0] and
            box.inst->getCenLoc().Mdist(boxCen) <= shellSearchSide * siteWidth and
            box.inst->getCLKPin(0)->getNet() == clkNet) {
            rest_candidates.emplace_back(box.inst);
        }
    }
    int rest_size{shape2Size[getboxShape(bestShell)] - 1};
    if ((int)rest_candidates.size() < rest_size) return;

    std::partial_sort(rest_candidates.begin(),
                      rest_candidates.begin() + rest_size,
                      rest_candidates.end(),
                      [&boxCen](Instance* a, Instance* b) {
                          return a->getCenLoc().Mdist(boxCen) <
                                 b->getCenLoc().Mdist(boxCen);
                      });

    for (int i = 0; i < rest_size; i++) {
        merge_ffs.emplace_back(rest_candidates[i]);
    }
}

Box Solver::findBestShell(const Instance* FF_inst, const FlipFlop* bestFitFF, double shellSearchSide) {
    const double searchRadius = shellSearchSide * siteWidth;
    Coord FF_cen_loc = FF_inst->getCenLoc();
    Box searchArea(
        FF_cen_loc - Coord(searchRadius * SQRT2, searchRadius * SQRT2),
        FF_cen_loc + Coord(searchRadius * SQRT2, searchRadius * SQRT2));

    std::vector<Box> touchedBoxes;
    FFShellBoxes.query(bgi::intersects(searchArea), std::back_inserter(touchedBoxes));

    Box bestShell(Coord(INT_MIN, INT_MIN),
                  Coord(INT_MIN, INT_MIN) + Coord(bestFitFF->getWidth(), bestFitFF->getHeight()));

    for (auto& box : touchedBoxes) {
        if (box.inst == nullptr) {
            if (getBoxCen(box).Mdist(FF_cen_loc) < getBoxCen(bestShell).Mdist(FF_cen_loc) and
                getBoxCen(box).Mdist(FF_cen_loc) <= searchRadius and
                shape2Size[getboxShape(box)] == bestFitFF->getBits()) {
                bestShell = box;
            }
        }
    }

    return bestShell;
}

void Solver::pinSwap(Instance* FF_inst) {
    int n = FF_inst->getInputPins().size();
    assert(n > 0 && (n & (n - 1)) == 0);  // verify n is power of 2
    int rdm_pin_idx = (distr1000(generator) & (n - 1));

    Coord FF_center = FF_inst->getCenLoc();

    double pinSwapRadius = PINSWAP_SIDE * siteWidth;
    Box searchArea(
        FF_center - Coord(pinSwapRadius * SQRT2, pinSwapRadius * SQRT2),
        FF_center + Coord(pinSwapRadius * SQRT2, pinSwapRadius * SQRT2));

    std::vector<Box> touchedBoxes;
    allInstBoxes.query(bgi::intersects(searchArea), std::back_inserter(touchedBoxes));

    using PinInfo = std::pair<Instance*, int>;
    std::vector<PinInfo> swapPinCddts;
    for (auto& box : touchedBoxes) {
        if (box.inst->isType(Cell::Type::FF) and
            box.inst->getCenLoc().Mdist(FF_center) <= pinSwapRadius and
            box.inst != FF_inst and
            box.inst->getCLKPin(0)->getNet() == FF_inst->getCLKPin(0)->getNet()) {
            int nearbySize = box.inst->getInputPins().size();
            for (int i = 0; i < nearbySize; ++i) {
                swapPinCddts.emplace_back(box.inst, i);
            };
        }
    }
    if (swapPinCddts.empty()) return;

    PinInfo swapPin = swapPinCddts[0];
    std::shuffle(swapPinCddts.begin(), swapPinCddts.end(), generator);

    auto selfInPin = FF_inst->getInputPin(rdm_pin_idx);
    auto selfOutPin = FF_inst->getOutputPin(rdm_pin_idx);
    double curMaxD2O{0};
    for (auto& self_drain : selfOutPin->getNet()->getDrainPins()) {
        curMaxD2O = std::max(curMaxD2O, selfOutPin->Mdist(self_drain));
    }

    double bestTNS = DBL_MAX;
    auto selfSrc = selfInPin->getNet()->getSrcPin();
    for (auto& [inst, pin_idx] : swapPinCddts) {
        auto otherInPin = inst->getInputPin(pin_idx);
        auto otherOutPin = inst->getOutputPin(pin_idx);
        auto otherSrc = otherInPin->getNet()->getSrcPin();

        double curMaxDist2Out{curMaxD2O}, newMaxDist2Out{0};

        for (auto& self_drain : selfOutPin->getNet()->getDrainPins()) {
            newMaxDist2Out = std::max(newMaxDist2Out, otherOutPin->Mdist(self_drain));
        }

        for (auto& other_drain : otherOutPin->getNet()->getDrainPins()) {
            curMaxDist2Out = std::max(curMaxDist2Out, otherOutPin->Mdist(other_drain));
            newMaxDist2Out = std::max(newMaxDist2Out, selfOutPin->Mdist(other_drain));
        }

        double curDist2In = selfInPin->Mdist(selfSrc) + otherInPin->Mdist(otherSrc);
        double newDist2In = selfInPin->Mdist(otherSrc) + otherInPin->Mdist(selfSrc);

        if (newDist2In + newMaxDist2Out >= curDist2In + curMaxDist2Out) {
            continue;
        }

        double TNS = cellMgr->getSwapTNS(FF_inst, inst, rdm_pin_idx, pin_idx);
        if (TNS < bestTNS) {
            bestTNS = TNS;
            swapPin = {inst, pin_idx};
        }
    }

    if (bestTNS == DBL_MAX) return;

    FF_inst->swapPin(swapPin.first, rdm_pin_idx, swapPin.second);
}
