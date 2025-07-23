#ifndef PIN_PROPERTY_HPP
#define PIN_PROPERTY_HPP

#include <string>
#include <unordered_map>

class PinProperty 
{
private:
    std::string pinName;
    std::unordered_map<std::string, std::string> properties;

public:
    // Constructor
    PinProperty() {}
    PinProperty(const std::string& pinName) : pinName(pinName) {}
    PinProperty(const std::string& pinName, const std::unordered_map<std::string, std::string>& properties)
        : pinName(pinName), properties(properties) {}

    // Getters
    const std::string& getPinName() const { return pinName; }
    const std::unordered_map<std::string, std::string>& getProperties() const { return properties; }
    
    // Setters
    void setPinName(const std::string& pinName) { this->pinName = pinName; }
    void setProperties(const std::unordered_map<std::string, std::string>& properties) { this->properties = properties; }
    
    // Property management methods
    void addProperty(const std::string& key, const std::string& value) { properties[key] = value; }
    void removeProperty(const std::string& key) { properties.erase(key); }
    void clearProperties() { properties.clear(); }
    
    bool hasProperty(const std::string& key) const { return properties.find(key) != properties.end(); }
    std::string getProperty(const std::string& key) const {
        auto it = properties.find(key);
        return (it != properties.end()) ? it->second : "";
    }
    
    size_t getPropertyCount() const { return properties.size(); }
};

#endif // PIN_PROPERTY_HPP