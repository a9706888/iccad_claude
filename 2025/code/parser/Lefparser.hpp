#pragma once
#include <string>
#include <vector>
#include "../structure/Macro/Macro.hpp"
#include "../structure/Obs/Obs.hpp"
#include "../structure/LEFPin/LEFPin.hpp"
#include "../structure/Port/Port.hpp"
#include "../structure/Geometry/Geometry.hpp"

class LEFParser 
{
public:
    std::vector<Macro> macros;

    bool parse(const std::string &lef_file);
    void printSummary() const;

private:
    static void trim(std::string &s);
};