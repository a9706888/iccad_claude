#include "global.hpp"

#include <iostream>

// Definition
Timer timer;
Evaluator eval = Evaluator();

const int RDMSHIFTTIMES = 100;
const int MOVE_SIDE = 40;

const int PINSWAP_SIDE = 250;
int SHELLSEARCH_SIDE = 300;

const double CNVRGTHRESHOLD = 0.005;

void printParas() {
    std::cout << "============= " << "Merging Parameters" << " =============\n"
              << "RDMSHIFTTIMES: " << RDMSHIFTTIMES << "\n"
              << "MOVE_SIDE: " << MOVE_SIDE << "\n"
              << "PINSWAP_SIDE: " << PINSWAP_SIDE << "\n"
              << "SHELLSEARCH_SIDE: " << SHELLSEARCH_SIDE << "\n"
              << "CNVRGTHRESHOLD: " << CNVRGTHRESHOLD << "\n"
              << "============================================\n";
}
