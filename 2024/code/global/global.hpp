#pragma once
#include "../Evaluator/Evaluator.hpp"
#include "../Timer/Timer.hpp"
// #define DEBUG_MODE

// Declaration
extern Timer timer;
extern Evaluator eval;


extern const int RDMSHIFTTIMES;
extern const int MOVE_SIDE;
extern const int PINSWAP_SIDE;
extern int SHELLSEARCH_SIDE;


extern const double CNVRGTHRESHOLD;

extern void printParas();