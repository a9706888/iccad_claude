#ifndef DEF_PARSER_H
#define DEF_PARSER_H

#include <string>
#include <vector>
#include <unordered_map>

#include "../structure/Row/Row.hpp"
#include "../structure/Component/Component.hpp"
#include "../structure/Pin/Pin.hpp"
#include "../structure/Net/Net.hpp"
#include "../structure/Track/Track.hpp"
#include "../structure/Pinproperty/Pinproperty.hpp"

class DEFParser 
{
public:
    std::string version;
    std::string designName;
    unsigned long long dieAreaX, dieAreaY;
    std::vector<Component> components;
    std::vector<Pin> pins;
    std::vector<Net> nets;
    std::vector<Row> rows;
    std::vector<Track> tracks;
    std::vector<PinProperty> pinProperties;

    char dividerChar = '/';
    std::string busBitChars = "[]";
    int unitDesign = 1000; // Default unit for distance in microns

    bool parseFile(const std::string &filename);
    void printSummary();

private:
    void parseDieArea(const std::string &line);
    void parseRowLine(const std::string &line);
    void parseTrackLine(const std::string &line);
    bool parsePinProperties(std::ifstream &file, int count);
    void parseComponents(std::ifstream &file, int count);
    void parsePins(std::ifstream &file, int count);
    void parseNets(std::ifstream &file, int netCount);
};

#endif