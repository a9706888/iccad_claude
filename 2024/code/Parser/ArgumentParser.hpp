#pragma once
#include <unistd.h>

#include <iostream>
#include <string>

class ArgumentParser {
   public:
    std::string inputPath, outputPath;

    ArgumentParser() {}
    ~ArgumentParser() {};

    void parse(int argc, char *argv[]) {
        int opt;
        while ((opt = getopt(argc, argv, "h")) != -1) {
            switch (opt) {
                default:
                    std::cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
                    exit(1);
                    break;
            }
        }

        if (argc - optind != 2) {
            std::cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
            exit(1);
        }
        inputPath = argv[optind];
        outputPath = argv[optind + 1];
    }
};
