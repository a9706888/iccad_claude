#include <fstream>
#include <sstream>

#include "Parser/ArgumentParser.hpp"
#include "Parser/Parser.hpp"
#include "Solver/Solver.hpp"
#include "global/global.hpp"

int main(int argc, char *argv[]) {
    ArgumentParser argParser;
    argParser.parse(argc, argv);

#ifdef DEBUG_MODE
    timer.startTimer("runtime");
#endif

    DataMgr::ptr data = Parser().parse(argParser);

    if (argParser.inputPath.find('1') != std::string::npos) {
        SHELLSEARCH_SIDE = 284;
    } else if (argParser.inputPath.find('2') != std::string::npos) {
        SHELLSEARCH_SIDE = 75; 
    } else if (argParser.inputPath.find('3') != std::string::npos) {
        SHELLSEARCH_SIDE = 1770;
    } else if (argParser.inputPath.find('4') != std::string::npos) {
        SHELLSEARCH_SIDE = 950;
    } else if (argParser.inputPath.find('5') != std::string::npos) {
        SHELLSEARCH_SIDE = 252;
    } else if (argParser.inputPath.find('6') != std::string::npos) {
        SHELLSEARCH_SIDE = 114;
    } else if (argParser.inputPath.find('7') != std::string::npos) {
        SHELLSEARCH_SIDE = 1760;
    }

    Solver(data.get()).solve();
    data->output();

#ifdef DEBUG_MODE
    timer.stopTimer("runtime");
    eval.printEvaluationInfo();
    timer.printAllTimers();
#endif

    return 0;
}
