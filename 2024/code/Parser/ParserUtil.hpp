#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

class ParserUtil {
   public:
    using ptr = std::unique_ptr<ParserUtil>;
    std::string line;
    std::istringstream iss;
    std::ifstream fin;
    std::string identifier;

    ParserUtil(const std::string &inputPath) : fin(inputPath) {
        if (!fin.is_open()) {
            std::cerr << "[Error] Cannot open \"" << inputPath << "\".\n";
            exit(EXIT_FAILURE);
        }
        line = "";
    }

    void getIss() {
        do {
            iss.clear();
            iss.str("");
            getline(fin, line);
            iss.str(line);

        } while (readEmptyLine() and fin);
    }
    bool readEmptyLine() {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        return line.size() == 0;
    }
    void toLowerCase() {
        std::transform(identifier.begin(), identifier.end(), identifier.begin(), ::tolower);
    }

    ~ParserUtil() {
        fin.close();
    }
};
