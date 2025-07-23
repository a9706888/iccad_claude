#include "Weightparser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

// Constructor
WeightParser::WeightParser() {
    // Initialize as empty
}

// Destructor
WeightParser::~WeightParser() {
    // Clean up resources (if needed)
}

// Load weight data from file
bool WeightParser::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return false;
    }
    
    // Clear existing data
    weightMap.clear();
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        // Parse each line using stringstream
        std::stringstream ss(line);
        std::string name;
        double value;
        
        // Read name and value
        if (ss >> name >> value) {
            weightMap[name] = value;
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
        }
    }
    
    file.close();
    
    if (weightMap.empty()) {
        std::cerr << "No valid weight data found in file" << std::endl;
        return false;
    }
    
    std::cout << "Successfully loaded " << weightMap.size() << " weight values" << std::endl;
    return true;
}

// Get all weight data
const std::map<std::string, double>& WeightParser::getWeights() const {
    return weightMap;
}

// Get weight by name
double WeightParser::getWeight(const std::string& name) const {
    auto it = weightMap.find(name);
    if (it != weightMap.end()) {
        return it->second;
    }
    
    std::cerr << "Weight not found: " << name << std::endl;
    return 0.0;
}

// Get weight count
int WeightParser::getWeightCount() const {
    return weightMap.size();
}

// Print all weights
void WeightParser::printWeights() const 
{
    if (weightMap.empty()) 
    {
        std::cout << "No weight data" << std::endl;
        return;
    }

    std::cout << "Weight data:" << std::endl;
    for (const auto& pair : weightMap) 
    {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
}

// Clear data
void WeightParser::clear() {
    weightMap.clear();
}

// Check if data is empty
bool WeightParser::isEmpty() const {
    return weightMap.empty();
}

// Check if contains weight with specific name
bool WeightParser::hasWeight(const std::string& name) const 
{
    return weightMap.find(name) != weightMap.end();
}

// Get all weight names
std::vector<std::string> WeightParser::getWeightNames() const 
{
    std::vector<std::string> names;
    for (const auto& pair : weightMap) {
        names.push_back(pair.first);
    }
    return names;
}