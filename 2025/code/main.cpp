//  g++ main.cpp parser/*.cpp -std=c++17 -O3 -o cadb_1060
// ./cadb_1060 -weight ._weight lib -lib -lef _.lef -db -.db -tf _.tf -sdc _.sdc -v -.v -def _.def -out outputname

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "parser/Defparser.hpp"
#include "parser/Weightparser.hpp"
#include "parser/LEFParser.hpp"   

int main(int argc, char** argv) {
  std::unordered_map<std::string, std::vector<std::string>> files;
  std::vector<std::string> flags = {
    "-weight","-lib","-lef","-db","-tf","-sdc","-v","-def"
  };
  std::string outName;

  for(int i=1; i<argc; ++i) {
    std::string a = argv[i];
    if (a=="-out") {
      if (i+1<argc) {
        outName = argv[++i];
      }
    }
    else if (std::find(flags.begin(), flags.end(), a) != flags.end()) {
      auto &vec = files[a];
      while(i+1<argc && argv[i+1][0] != '-') {
        vec.emplace_back(argv[++i]);
      }
    }
    else {
      std::cerr << "Unknown option: " << a << "\n";
      return 1;
    }
  }

  std::string wfile  = files["-weight"].empty() ? "" : files["-weight"][0];
  std::string dfile  = files["-def"].empty()    ? "" : files["-def"][0];
  auto       lefs    = files["-lef"];   

  WeightParser wp;
  if (!wfile.empty() && wp.loadFromFile(wfile)) {
    wp.printWeights();
  } else {
    std::cout<<"Warning: no weight loaded\n";
  }

  DEFParser dp;
  if (!dfile.empty() && dp.parseFile(dfile)) {
    dp.printSummary();
  } else {
    std::cout<<"Warning: no DEF loaded\n";
  }

  LEFParser lp;
  for (auto &lf: lefs) {
    if (lp.parse(lf)) {
      lp.printSummary();
    } else {
      std::cerr<<"Warning: failed to parse LEF "<<lf<<"\n";
    }
  }

  if (!outName.empty()) {
    std::cout<<"Will write results into "<<outName<<".*\n";
  }

  return 0;
}
