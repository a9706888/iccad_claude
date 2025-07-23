#ifndef WEIGHTPARSER_H
#define WEIGHTPARSER_H

#include <string>
#include <vector>
#include <map>
#include <iostream>

class WeightParser {
private:
    std::map<std::string, double> weightMap;  // Store name-value pairs
    
public:
    // Constructor
    WeightParser();
    
    // Destructor
    ~WeightParser();
    
    // Load weight data from file
    bool loadFromFile(const std::string& filename);
    
    // Get all weight data
    const std::map<std::string, double>& getWeights() const;
    
    // Get weight by name
    double getWeight(const std::string& name) const;
    
    // Get weight count
    int getWeightCount() const;
    
    // Print all weights
    void printWeights() const;
    
    // Clear data
    void clear();
    
    // Check if data is empty
    bool isEmpty() const;
    
    // Check if contains weight with specific name
    bool hasWeight(const std::string& name) const;
    
    // Get all weight names
    std::vector<std::string> getWeightNames() const;
};

#endif // WEIGHTPARSER_H