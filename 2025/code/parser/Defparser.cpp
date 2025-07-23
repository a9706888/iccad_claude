#include "Defparser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>

// 記得要 include 新的 class header files
#include "../structure/Row/Row.hpp"
#include "../structure/Component/Component.hpp"
#include "../structure/Pin/Pin.hpp"
#include "../structure/Net/Net.hpp"
#include "../structure/Track/Track.hpp"
#include "../structure/Pinproperty/Pinproperty.hpp"

// Main parsing function
bool DEFParser::parseFile(const std::string &filename) 
{
    std::ifstream file(filename);
    if (!file.is_open()) 
    {
        std::cerr << "Can't open file: " << filename << std::endl;
        return false;
    }
    std::string line;
    while (std::getline(file, line)) 
    {
        if (line.empty()) continue;

        std::stringstream ss(line);

        if (line.find("VERSION") == 0) {
            ss >> version >> version;
        } else if (line.find("DIVIDERCHAR") == 0) {
            std::string keyword, symbol;
            ss >> keyword >> symbol;
            if (symbol.length() >= 3 && symbol.front() == '"' && symbol.back() == '"') {
                dividerChar = symbol[1];
            }
        } else if (line.find("BUSBITCHARS") == 0) {
            std::string keyword, symbol;
            ss >> keyword >> symbol;
            if (symbol.length() >= 4 && symbol.front() == '"' && symbol.back() == '"') {
                busBitChars = symbol.substr(1, symbol.length() - 2);
            }
        } else if (line.find("DESIGN") == 0) {
            ss >> line >> designName;
        } else if (line.find("UNITS DISTANCE MICRONS") == 0) {
            std::string keyword;
            ss >> keyword >> keyword >> keyword >> unitDesign;
        } else if (line.find("PROPERTYDEFINITIONS") == 0) {
            while (std::getline(file, line)) {
                if (line.find("END PROPERTYDEFINITIONS") != std::string::npos)
                    break;
            }
        } else if (line.find("DIEAREA") == 0) {
            parseDieArea(line);
        } else if (line.find("ROW") == 0) {
            parseRowLine(line);
        } else if (line.find("TRACKS") == 0) {
            parseTrackLine(line);
        } else if (line.find("PINPROPERTIES") == 0) {
            std::stringstream ss(line);
            std::string keyword;
            int pinPropCount = 0;
            ss >> keyword >> pinPropCount;
            parsePinProperties(file, pinPropCount);
        } else if (line.find("COMPONENTS") == 0) {
            std::stringstream ss(line);
            std::string keyword;
            int compCount = 0;
            ss >> keyword >> compCount;
            parseComponents(file, compCount);
        } else if (line.find("PINS") == 0) {
            std::stringstream ss(line);
            std::string keyword;
            int pinCount = 0;
            ss >> keyword >> pinCount;
            parsePins(file, pinCount);
        } else if (line.find("NETS") == 0) {
            std::stringstream ss(line);
            std::string keyword;
            int netCount = 0;
            ss >> keyword >> netCount;
            parseNets(file, netCount);
        }
    }
    return true;
}

// --- Helper Functions ---
void DEFParser::parseDieArea(const std::string &line) {
    std::regex regex_diearea(R"(\(\s*(\d+)\s+(\d+)\s*\))");
    std::smatch match;
    std::string::const_iterator start = line.cbegin();
    std::vector<std::pair<int, int>> coords;

    while (std::regex_search(start, line.cend(), match, regex_diearea)) {
        coords.push_back({std::stoi(match[1]), std::stoi(match[2])});
        start = match.suffix().first;
    }

    if (!coords.empty()) {
        dieAreaX = 0;
        dieAreaY = 0;
        for (const auto &coord : coords) {
            dieAreaX = std::max(dieAreaX, (unsigned long long)coord.first);
            dieAreaY = std::max(dieAreaY, (unsigned long long)coord.second);
        }
    }
}

void DEFParser::parseRowLine(const std::string &line) {
    Row row;
    std::stringstream ss(line);
    std::string tmp, name, site, orientation;
    int x, y, numX, numY, stepX, stepY;
    
    ss >> tmp >> name >> site >> x >> y >> orientation;
    ss >> tmp >> numX >> tmp >> numY >> tmp >> stepX >> stepY;
    
    // 使用 setter 方法設定 class 成員
    row.setName(name);
    row.setSite(site);
    row.setPosition(x, y);
    row.setOrientation(orientation);
    row.setNumber(numX, numY);
    row.setStep(stepX, stepY);
    
    rows.push_back(row);
}

void DEFParser::parseTrackLine(const std::string &line) {
    Track track;
    std::stringstream ss(line);
    std::string tmp, direction, layer;
    int start, number, step;
    
    ss >> tmp >> direction >> start >> tmp >> number >> tmp >> step >> tmp >> layer;
    if (!layer.empty() && layer.back() == ';') layer.pop_back();
    
    // 使用 setter 方法設定 Track class 成員
    track.setDirection(direction);
    track.setStart(start);
    track.setNumber(number);
    track.setStep(step);
    track.setLayer(layer);
    
    tracks.push_back(track);
}

bool DEFParser::parsePinProperties(std::ifstream &file, int count) {
    std::string line;
    PinProperty currentPinProp;
    int parsedCount = 0;

    while (parsedCount < count && std::getline(file, line)) {
        if (line.find("END PINPROPERTIES") != std::string::npos) {
            if (!currentPinProp.getPinName().empty()) {
                pinProperties.push_back(currentPinProp);
                parsedCount++;
            }
            break;
        }

        std::smatch matchPin;
        std::regex regexPin(R"(^\s*-\s*PIN\s+(\S+))");
        if (std::regex_search(line, matchPin, regexPin)) {
            if (!currentPinProp.getPinName().empty()) {
                pinProperties.push_back(currentPinProp);
                parsedCount++;
            }
            currentPinProp = PinProperty();
            currentPinProp.setPinName(matchPin[1]);
            continue;
        }

        std::smatch matchProp;
        std::regex regexProp(R"(^\s*\+\s*PROPERTY\s+(\S+)\s+\"([^\"]+)\")");
        if (std::regex_search(line, matchProp, regexProp)) {
            std::string propName = matchProp[1];
            std::string propValue = matchProp[2];
            currentPinProp.addProperty(propName, propValue);
        }
    }

    if (!currentPinProp.getPinName().empty() && parsedCount < count) {
        pinProperties.push_back(currentPinProp);
    }
    return true;
}

void DEFParser::parseComponents(std::ifstream &file, int count) {
    std::string line;
    int parsedCount = 0;
    while (parsedCount < count && std::getline(file, line)) {
        if (line.find("END COMPONENTS") != std::string::npos)
            break;
        if (line.empty()) continue;

        if (line.find(" - ") != std::string::npos || line.find("- ") == 0) {
            Component comp;
            std::regex comp_regex(R"(^\s*-\s+(\S+)\s+(\S+)\s+\+\s+PLACED\s+\(\s*(\d+)\s+(\d+)\s*\)\s+(\S+))");
            std::smatch match;
            if (std::regex_search(line, match, comp_regex)) {
                // 使用 setter 方法設定 Component class 成員
                comp.setName(match[1]);
                comp.setMacro(match[2]);
                comp.setPosition(std::stoi(match[3]), std::stoi(match[4]));
                comp.setOrientation(match[5]);
                
                components.push_back(comp);
                parsedCount++;
            }
        }
    }
}

void DEFParser::parsePins(std::ifstream &file, int count) {
    std::string line;
    int parsedCount = 0;
    while (parsedCount < count && std::getline(file, line)) {
        if (line.find("END PINS") != std::string::npos) break;
        if (line.empty()) continue;

        if (line.find(" - ") != std::string::npos || line.find("- ") == 0) {
            Pin pin;
            std::regex pin_regex(R"(^\s*-\s+(\S+)\s+\+\s+NET\s+(\S+))");
            std::smatch match;
            if (std::regex_search(line, match, pin_regex)) {
                // 使用 setter 方法設定 Pin class 成員
                pin.setName(match[1]);
                pin.setNet(match[2]);
                
                while (std::getline(file, line)) {
                    if (line.find("PLACED") != std::string::npos) {
                        std::regex placed_regex(R"(PLACED\s+\(\s*(\d+)\s+(\d+)\s*\))");
                        std::smatch placed_match;
                        if (std::regex_search(line, placed_match, placed_regex)) {
                            pin.setPosition(std::stoi(placed_match[1]), std::stoi(placed_match[2]));
                            pins.push_back(pin);
                            parsedCount++;
                            break;
                        }
                    }
                    if (line.find("- ") == 0 || line.find("END PINS") != std::string::npos) {
                        break;
                    }
                }
            }
        }
    }
}

void DEFParser::parseNets(std::ifstream &file, int netCount) {
    std::string line;
    Net currentNet;
    int parsedCount = 0;
    bool readingNet = false;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        if (line.find("END NETS") != std::string::npos) {
            if (readingNet && !currentNet.getName().empty()) {
                nets.push_back(currentNet);
                parsedCount++;
            }
            break;
        }

        // 新的 net 開始
        if (line.find("- ") == 0 || line.find(" - ") != std::string::npos) {
            if (readingNet && !currentNet.getName().empty()) {
                nets.push_back(currentNet);
                parsedCount++;
            }

            readingNet = true;
            currentNet = Net();

            // 解析 net 名稱
            std::stringstream ss(line);
            std::string tmp, netName;
            ss >> tmp >> netName; // "- netName"
            currentNet.setName(netName);
            continue;
        }

        if (readingNet) {
            // 解析 net 內的連接 ( inst pin )
            std::regex conn_regex(R"(\(\s*(\S+)\s+(\S+)\s*\))");
            std::smatch match;
            std::string::const_iterator start = line.cbegin();

            while (std::regex_search(start, line.cend(), match, conn_regex)) {
                std::string instance = match[1];
                std::string pin = match[2];
                currentNet.addConnection(instance + ":" + pin);
                start = match.suffix().first;
            }

            // 如果行中有 `;`，表示這個 net 結束
            if (line.find(';') != std::string::npos) {
                nets.push_back(currentNet);
                parsedCount++;
                readingNet = false;
            }
        }

        if (parsedCount >= netCount) break;
    }
}

void DEFParser::printSummary() {
    std::cout << "=== PARSING SUMMARY ===\n";
    std::cout << "Design: " << designName << "\n";
    std::cout << "Version: " << version << "\n";
    std::cout << "Die Area: " << dieAreaX << " x " << dieAreaY << "\n";
    std::cout << "Unit Design: " << unitDesign << "\n";
    std::cout << "Rows: " << rows.size() << "\n";
    std::cout << "Tracks: " << tracks.size() << "\n";
    std::cout << "Components: " << components.size() << "\n";
    std::cout << "Pins: " << pins.size() << "\n";
    std::cout << "Nets: " << nets.size() << "\n";
    std::cout << "Pin Properties: " << pinProperties.size() << "\n";
}