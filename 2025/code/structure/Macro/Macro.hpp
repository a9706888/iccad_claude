#ifndef MACRO_HPP
#define MACRO_HPP

#include <string>
#include <vector>
#include <algorithm>
#include "../LEFPin/LEFPin.hpp"
#include "../Obs/Obs.hpp"

class Macro {
private:
    std::string name;
    std::string cls1, cls2;
    double originX, originY;
    double sizeX, sizeY;
    std::vector<std::string> symmetry;
    std::string site;
    std::vector<LEFPin> pins;
    std::vector<Obs> obss;

public:
    // Constructor
    Macro() : originX(0), originY(0), sizeX(0), sizeY(0) {}
    Macro(const std::string& name) : name(name), originX(0), originY(0), sizeX(0), sizeY(0) {}

    // Getters
    const std::string& getName() const { return name; }
    const std::string& getClass1() const { return cls1; }
    const std::string& getClass2() const { return cls2; }
    double getOriginX() const { return originX; }
    double getOriginY() const { return originY; }
    double getSizeX() const { return sizeX; }
    double getSizeY() const { return sizeY; }
    const std::vector<std::string>& getSymmetry() const { return symmetry; }
    const std::string& getSite() const { return site; }
    const std::vector<LEFPin>& getPins() const { return pins; }
    std::vector<LEFPin>& getPins() { return pins; }
    const std::vector<Obs>& getObstructions() const { return obss; }
    std::vector<Obs>& getObstructions() { return obss; }
    size_t getPinCount() const { return pins.size(); }
    size_t getObsCount() const { return obss.size(); }

    // Setters
    void setName(const std::string& name) { this->name = name; }
    void setClass1(const std::string& cls1) { this->cls1 = cls1; }
    void setClass2(const std::string& cls2) { this->cls2 = cls2; }
    void setOriginX(double originX) { this->originX = originX; }
    void setOriginY(double originY) { this->originY = originY; }
    void setOrigin(double x, double y) { originX = x; originY = y; }
    void setSizeX(double sizeX) { this->sizeX = sizeX; }
    void setSizeY(double sizeY) { this->sizeY = sizeY; }
    void setSize(double x, double y) { sizeX = x; sizeY = y; }
    void setSymmetry(const std::vector<std::string>& symmetry) { this->symmetry = symmetry; }
    void setSite(const std::string& site) { this->site = site; }
    void setPins(const std::vector<LEFPin>& pins) { this->pins = pins; }
    void setObstructions(const std::vector<Obs>& obss) { this->obss = obss; }

    // 對稱性管理
    void addSymmetry(const std::string& sym) { symmetry.push_back(sym); }
    void clearSymmetry() { symmetry.clear(); }
    bool hasSymmetry(const std::string& sym) const {
        return std::find(symmetry.begin(), symmetry.end(), sym) != symmetry.end();
    }

    // Pin 管理方法
    void addPin(const LEFPin& pin) { pins.push_back(pin); }
    void addPin(const std::string& name, const std::string& direction, const std::string& use) {
        pins.emplace_back(name, direction, use);
    }
    void clearPins() { pins.clear(); }
    
    LEFPin* findPin(const std::string& pinName) {
        for (auto& pin : pins) {
            if (pin.getName() == pinName) {
                return &pin;
            }
        }
        return nullptr;
    }
    
    const LEFPin* findPin(const std::string& pinName) const {
        for (const auto& pin : pins) {
            if (pin.getName() == pinName) {
                return &pin;
            }
        }
        return nullptr;
    }

    // 障礙物管理方法
    void addObstruction(const Obs& obs) { obss.push_back(obs); }
    void addObstruction(const std::string& layer) {
        obss.emplace_back(layer);
    }
    void clearObstructions() { obss.clear(); }

    // 實用方法
    double getArea() const { return sizeX * sizeY; }
    bool isEmpty() const { return sizeX <= 0 || sizeY <= 0; }
    
    // 統計方法
    size_t getInputPinCount() const {
        size_t count = 0;
        for (const auto& pin : pins) {
            if (pin.isInput()) count++;
        }
        return count;
    }
    
    size_t getOutputPinCount() const {
        size_t count = 0;
        for (const auto& pin : pins) {
            if (pin.isOutput()) count++;
        }
        return count;
    }
    
    size_t getInOutPinCount() const {
        size_t count = 0;
        for (const auto& pin : pins) {
            if (pin.isInOut()) count++;
        }
        return count;
    }
    
    // 獲取 Macro 的邊界框
    Rect getBoundingBox() const {
        return Rect(originX, originY, originX + sizeX, originY + sizeY);
    }
    
    // 檢查是否為特定類型的 Macro
    bool isCore() const { return cls1 == "CORE" || cls2 == "CORE"; }
    bool isPad() const { return cls1 == "PAD" || cls2 == "PAD"; }
    bool isCover() const { return cls1 == "COVER" || cls2 == "COVER"; }
};

#endif // MACRO_HPP