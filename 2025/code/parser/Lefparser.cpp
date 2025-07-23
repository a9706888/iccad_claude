#include "Lefparser.hpp"
#include "../structure/Macro/Macro.hpp"
#include "../structure/LEFPin/LEFPin.hpp"
#include "../structure/Port/Port.hpp"
#include "../structure/Obs/Obs.hpp"
#include "../structure/Geometry/Geometry.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

void LEFParser::trim(std::string &s) {
  const char* ws = " \t\r\n";
  size_t b = s.find_first_not_of(ws);
  size_t e = s.find_last_not_of(ws);
  s = (b==std::string::npos ? "" : s.substr(b,e-b+1));
}

bool LEFParser::parse(const std::string &lef_file) {
  std::ifstream f(lef_file);
  if (!f.is_open()) {
    std::cerr<<"Cannot open "<<lef_file<<"\n";
    return false;
  }

  enum State { ST_GLOBAL, ST_MACRO, ST_PIN, ST_PORT, ST_OBS } state=ST_GLOBAL;
  Macro *curMacro=nullptr;
  LEFPin *curPin=nullptr;
  Port *curPort=nullptr;
  Obs *curObs=nullptr;

  std::string line;
  while (std::getline(f,line)) {
    trim(line);
    if (line.empty() || line[0]=='#') continue;

    std::istringstream ss(line);
    if (state==ST_GLOBAL) {
      if (line.rfind("MACRO ",0)==0) {
        macros.emplace_back();
        curMacro = &macros.back();
        // MACRO <name>
        ss.ignore(5);
        std::string macroName;
        ss>>macroName;
        curMacro->setName(macroName);
        state = ST_MACRO;
      }
    }
    else if (state==ST_MACRO) {
      if (line.rfind("CLASS ",0)==0) {
        ss.ignore(5);
        std::string cls1, cls2;
        ss>>cls1>>cls2;
        curMacro->setClass1(cls1);
        curMacro->setClass2(cls2);
      }
      else if (line.rfind("ORIGIN ",0)==0) {
        ss.ignore(6);
        double originX, originY;
        ss>>originX>>originY;
        curMacro->setOrigin(originX, originY);
      }
      else if (line.rfind("SIZE ",0)==0) {
        ss.ignore(4);
        double sizeX, sizeY;
        ss>>sizeX;
        ss.ignore(3); // " BY"
        ss>>sizeY;
        curMacro->setSize(sizeX, sizeY);
      }
      else if (line.rfind("SYMMETRY ",0)==0) {
        ss.ignore(8);
        std::string sym;
        std::vector<std::string> symmetries;
        while (ss>>sym) {
          if (sym.back()==';') {
            sym.pop_back();
            symmetries.push_back(sym);
            break;
          }
          symmetries.push_back(sym);
        }
        curMacro->setSymmetry(symmetries);
      }
      else if (line.rfind("SITE ",0)==0) {
        ss.ignore(5);
        std::string site;
        ss>>site;
        curMacro->setSite(site);
      }
      else if (line.rfind("PIN ",0)==0) {
        // 創建新的 Pin 並添加到 Macro
        LEFPin newPin;
        ss.ignore(4);
        std::string pinName;
        ss>>pinName;
        newPin.setName(pinName);
        curMacro->addPin(newPin);
        
        // 獲取剛添加的 Pin 的指針
        curPin = curMacro->findPin(pinName);
        state = ST_PIN;
      }
      else if (line=="OBS") {
        // 創建新的 Obs 並添加到 Macro
        Obs newObs;
        curMacro->addObstruction(newObs);
        
        // 獲取剛添加的 Obs 的指針
        curObs = &curMacro->getObstructions().back();
        state = ST_OBS;
      }
      else if (line.rfind("END ",0)==0) {
        // END <macro_name>
        std::string tmp, name2;
        ss>>tmp>>name2;
        if (name2==curMacro->getName()) {
          curMacro = nullptr;
          state = ST_GLOBAL;
        }
      }
    }
    else if (state==ST_PIN) {
      if (line.rfind("DIRECTION ",0)==0) {
        ss.ignore(9);
        std::string direction;
        ss>>direction;
        curPin->setDirection(direction);
      }
      else if (line.rfind("USE ",0)==0) {
        ss.ignore(4);
        std::string use;
        ss>>use;
        curPin->setUse(use);
      }
      else if (line=="PORT") {
        // 創建新的 Port 並添加到 Pin
        Port newPort;
        curPin->addPort(newPort);
        
        // 獲取剛添加的 Port 的指針
        curPort = &const_cast<std::vector<Port>&>(curPin->getPorts()).back();
        state = ST_PORT;
      }
      else if (line.rfind("END ",0)==0) {
        std::string tmp, name2;
        ss>>tmp>>name2;
        if (name2==curPin->getName()) {
          curPin = nullptr;
          state = ST_MACRO;
        }
      }
    }
    else if (state==ST_PORT) {
      if (line.rfind("LAYER ",0)==0) {
        ss.ignore(6);
        std::string layer;
        ss>>layer;
        curPort->setLayer(layer);
      }
      else if (line.rfind("RECT ",0)==0) {
        ss.ignore(5);
        double x1, y1, x2, y2;
        ss>>x1>>y1>>x2>>y2;
        curPort->addRect(x1, y1, x2, y2);
      }
      else if (line.rfind("POLYGON ",0)==0) {
        ss.ignore(8);
        std::vector<double> points;
        double v;
        while (ss>>v) {
          points.push_back(v);
        }
        curPort->addPolygon(points);
      }
      else if (line=="END") {
        curPort = nullptr;
        state = ST_PIN;
      }
    }
    else if (state==ST_OBS) {
      if (line.rfind("LAYER ",0)==0) {
        ss.ignore(6);
        std::string layer = line.substr(6);
        curObs->setLayer(layer);
      }
      else if (line.rfind("RECT ",0)==0) {
        ss.ignore(5);
        double x1, y1, x2, y2;
        ss>>x1>>y1>>x2>>y2;
        curObs->addRect(x1, y1, x2, y2);
      }
      else if (line.rfind("POLYGON ",0)==0) {
        ss.ignore(8);
        std::vector<double> points;
        double v;
        while (ss>>v) {
          points.push_back(v);
        }
        curObs->addPolygon(points);
      }
      else if (line=="END") {
        curObs = nullptr;
        state = ST_MACRO;
      }
    }
  }

  return true;
}

void LEFParser::printSummary() const 
{
  size_t totalPins=0, totalObs=0;
  for (const auto &m : macros) {
    totalPins += m.getPinCount();
    totalObs += m.getObsCount();
  }
  std::cout
    <<"Parsed "<<macros.size()<<" MACRO(s)\n"
    <<"Total PINs: "<<totalPins<<"\n"
    <<"Total OBS: "<<totalObs<<"\n";
}